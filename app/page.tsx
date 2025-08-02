'use client';

import React, { useRef, useState, useEffect, useCallback } from 'react';
import { Canvas, useFrame } from '@react-three/fiber';
import { OrbitControls } from '@react-three/drei';
import { Quaternion, Vector3, Group, Euler } from 'three';
import * as THREE from 'three';

// Define type for Emscripten module instance with _solveCube export
interface WasmModule {
  _solveCube: (ptr: number) => number;
  _malloc: (size: number) => number;
  _free: (ptr: number) => void;
  stringToUTF8: (str: string, outPtr: number, maxBytesToWrite: number) => void;
  UTF8ToString: (ptr: number) => string;
  lengthBytesUTF8: (str: string) => number;
}

// Cubie position type
type CubiePosition = [number, number, number];

// Individual cubie component
interface CubieProps {
  position: CubiePosition;
  colors: (string | null)[];
  cubeRef: React.MutableRefObject<Group | null>;
}

function Cubie({ position, colors, cubeRef }: CubieProps) {
  const meshRef = useRef<Group>(null);
  
  useFrame(() => {
    if (meshRef.current && cubeRef.current) {
      // Apply the cube's rotation to this cubie
      meshRef.current.quaternion.copy(cubeRef.current.quaternion);
    }
  });

  const [x, y, z] = position;
  const size = 0.9;
  const gap = 0.05;
  const actualPos: [number, number, number] = [x * (size + gap), y * (size + gap), z * (size + gap)];

  return (
    <group ref={meshRef} position={actualPos}>
      {/* Black core cube */}
      <mesh>
        <boxGeometry args={[size, size, size]} />
        <meshStandardMaterial color="#1a1a1a" />
      </mesh>
      
      {/* Face stickers */}
      {colors.map((color, faceIndex) => {
        if (!color) return null;
        
        const positions: [number, number, number][] = [
          [size/2 + 0.01, 0, 0],  // Right (+X)
          [-size/2 - 0.01, 0, 0], // Left (-X)
          [0, size/2 + 0.01, 0],  // Top (+Y)
          [0, -size/2 - 0.01, 0], // Bottom (-Y)
          [0, 0, size/2 + 0.01],  // Front (+Z)
          [0, 0, -size/2 - 0.01], // Back (-Z)
        ];
        
        const rotations: [number, number, number][] = [
          [0, Math.PI/2, 0],   // Right
          [0, -Math.PI/2, 0],  // Left
          [-Math.PI/2, 0, 0],  // Top
          [Math.PI/2, 0, 0],   // Bottom
          [0, 0, 0],           // Front
          [0, Math.PI, 0],     // Back
        ];
        
        return (
          <mesh
            key={faceIndex}
            position={positions[faceIndex]}
            rotation={rotations[faceIndex]}
          >
            <planeGeometry args={[size * 0.85, size * 0.85]} />
            <meshStandardMaterial color={color} />
          </mesh>
        );
      })}
    </group>
  );
}

// Main Rubik's cube component
interface RubiksCubeProps {
  cubeState: CubieData[];
}

interface CubieData {
  position: CubiePosition;
  colors: (string | null)[];
}

function RubiksCube({ cubeState }: RubiksCubeProps) {
  const cubeRef = useRef<Group>(null);

  return (
    <group ref={cubeRef}>
      {cubeState.map((cubie, index) => (
        <Cubie
          key={index}
          position={cubie.position}
          colors={cubie.colors}
          cubeRef={cubeRef}
        />
      ))}
    </group>
  );
}

// Generate initial solved cube state
function generateSolvedCube(): CubieData[] {
  const cubies: CubieData[] = [];
  
  // Standard Rubik's cube colors
  const faceColors = {
    right: '#C41E3A',   // Red
    left: '#FF5800',    // Orange
    top: '#FFFFFF',     // White
    bottom: '#FFDF00',  // Yellow
    front: '#0051BA',   // Blue
    back: '#009E60',    // Green
  };

  for (let x = -1; x <= 1; x++) {
    for (let y = -1; y <= 1; y++) {
      for (let z = -1; z <= 1; z++) {
        // Skip the center piece (invisible)
        if (x === 0 && y === 0 && z === 0) continue;
        
        const colors: (string | null)[] = [null, null, null, null, null, null];
        
        // Assign colors based on position
        if (x === 1) colors[0] = faceColors.right;   // Right face
        if (x === -1) colors[1] = faceColors.left;   // Left face
        if (y === 1) colors[2] = faceColors.top;     // Top face
        if (y === -1) colors[3] = faceColors.bottom; // Bottom face
        if (z === 1) colors[4] = faceColors.front;   // Front face
        if (z === -1) colors[5] = faceColors.back;   // Back face
        
        cubies.push({
          position: [x, y, z],
          colors
        });
      }
    }
  }
  
  return cubies;
}

export default function CubeSolverPage() {
  const [scramble, setScramble] = useState<string>('');
  const [scrambling, setScrambling] = useState<boolean>(false);
  const [solving, setSolving] = useState<boolean>(false);
  const [solutionMoves, setSolutionMoves] = useState<string[]>([]);
  const [scrambleMoves, setScrambleMoves] = useState<string[]>([]);
  const [cubeState, setCubeState] = useState<CubieData[]>(generateSolvedCube());
  const wasmModule = useRef<WasmModule | null>(null);

  useEffect(() => {
    async function loadWasm() {
      try {
        const module = await import('../public/solver.js');
        const instance: WasmModule = await module.default({
          locateFile: (path: string) => `/solver.wasm`,
        });
        wasmModule.current = instance;
      } catch (err) {
        console.error('Failed to load WASM module:', err);
      }
    }

    loadWasm();
  }, []);

  const callSolveCube = useCallback(
    (inputStr: string): string => {
      if (!wasmModule.current) return '';
      const mod = wasmModule.current;

      const len = mod.lengthBytesUTF8(inputStr) + 1;
      const ptr = mod._malloc(len);
      mod.stringToUTF8(inputStr, ptr, len);

      const resultPtr = mod._solveCube(ptr);
      const resultStr = mod.UTF8ToString(resultPtr);

      mod._free(ptr);

      return resultStr;
    },
    []
  );

  function parseMoves(movesStr: string): string[] {
    return movesStr.trim().split(/\s+/).filter((m) => m.length > 0);
  }

  function parseMove(moveStr: string): { face: string; turns: number } {
    const face = moveStr[0];
    let turns = 1;
    if (moveStr.length > 1) {
      if (moveStr[1] === '2') turns = 2;
      else if (moveStr[1] === "'") turns = 3;
    }
    return { face, turns };
  }

  // Function to rotate cubies based on face and turns
  function rotateFace(state: CubieData[], face: string, turns: number): CubieData[] {
    const newState = [...state];
    
    // Define which cubies belong to each face
    const getFaceCubies = (face: string) => {
      switch (face) {
        case 'R': return newState.filter(cubie => cubie.position[0] === 1);
        case 'L': return newState.filter(cubie => cubie.position[0] === -1);
        case 'U': return newState.filter(cubie => cubie.position[1] === 1);
        case 'D': return newState.filter(cubie => cubie.position[1] === -1);
        case 'F': return newState.filter(cubie => cubie.position[2] === 1);
        case 'B': return newState.filter(cubie => cubie.position[2] === -1);
        default: return [];
      }
    };

    // Rotate positions around the axis
    const rotatePosition = (pos: CubiePosition, axis: string, turns: number): CubiePosition => {
      let [x, y, z] = pos;
      
      for (let i = 0; i < turns; i++) {
        switch (axis) {
          case 'x': // R, L faces
            [y, z] = [-z, y];
            break;
          case 'y': // U, D faces
            [x, z] = [z, -x];
            break;
          case 'z': // F, B faces
            [x, y] = [-y, x];
            break;
        }
      }
      
      return [x, y, z];
    };

    // Get axis for face
    const getAxis = (face: string): string => {
      switch (face) {
        case 'R':
        case 'L': return 'x';
        case 'U':
        case 'D': return 'y';
        case 'F':
        case 'B': return 'z';
        default: return 'y';
      }
    };

    // Adjust turns for opposite faces
    const adjustedTurns = (face === 'L' || face === 'D' || face === 'B') ? (4 - turns) % 4 : turns;
    
    const faceCubies = getFaceCubies(face);
    const axis = getAxis(face);
    
    // Create mapping of old positions to new positions
    const positionMap = new Map<string, CubiePosition>();
    faceCubies.forEach(cubie => {
      const oldPos = cubie.position;
      const newPos = rotatePosition(oldPos, axis, adjustedTurns);
      positionMap.set(oldPos.join(','), newPos);
    });

    // Apply rotation to the state
    newState.forEach(cubie => {
      const key = cubie.position.join(',');
      if (positionMap.has(key)) {
        cubie.position = positionMap.get(key)!;
        
        // Also rotate the colors on the cubie
        const originalColors = [...cubie.colors];
        for (let i = 0; i < adjustedTurns; i++) {
          switch (axis) {
            case 'x': // Rotate around X axis
              [cubie.colors[2], cubie.colors[4], cubie.colors[3], cubie.colors[5]] = 
              [cubie.colors[5], cubie.colors[2], cubie.colors[4], cubie.colors[3]];
              break;
            case 'y': // Rotate around Y axis
              [cubie.colors[0], cubie.colors[4], cubie.colors[1], cubie.colors[5]] = 
              [cubie.colors[4], cubie.colors[1], cubie.colors[5], cubie.colors[0]];
              break;
            case 'z': // Rotate around Z axis
              [cubie.colors[0], cubie.colors[2], cubie.colors[1], cubie.colors[3]] = 
              [cubie.colors[3], cubie.colors[0], cubie.colors[2], cubie.colors[1]];
              break;
          }
        }
      }
    });

    return newState;
  }

  function animateMoves(moves: string[], onComplete: () => void) {
    if (moves.length === 0) {
      onComplete();
      return;
    }

    let index = 0;
    const duration = 600; // ms per move

    function processNextMove() {
      if (index >= moves.length) {
        onComplete();
        return;
      }

      const move = moves[index];
      const { face, turns } = parseMove(move);
      
      // Apply the move to the cube state
      setCubeState(prevState => rotateFace(prevState, face, turns));
      
      index++;
      setTimeout(processNextMove, duration);
    }

    processNextMove();
  }

  // Handle scramble animation
  useEffect(() => {
    if (scrambleMoves.length === 0 || !scrambling) return;

    animateMoves(scrambleMoves, () => {
      setScrambling(false);
      setScrambleMoves([]);
    });
  }, [scrambleMoves, scrambling]);

  // Handle solution animation
  useEffect(() => {
    if (solutionMoves.length === 0 || !solving) return;

    animateMoves(solutionMoves, () => {
      setSolving(false);
      setSolutionMoves([]);
    });
  }, [solutionMoves, solving]);

  async function onScrambleClick() {
    if (!scramble) return;
    setScrambling(true);
    const moves = parseMoves(scramble);
    setScrambleMoves(moves);
  }

  async function onSolveClick() {
    if (!scramble || !wasmModule.current) return;
    setSolving(true);
    const solutionStr = callSolveCube(scramble);
    const moves = parseMoves(solutionStr);
    setSolutionMoves(moves);
  }

  function onResetClick() {
    setCubeState(generateSolvedCube());
    setScramble('');
    setScrambleMoves([]);
    setSolutionMoves([]);
    setScrambling(false);
    setSolving(false);
  }

  const isAnimating = scrambling || solving;

  return (
    <div style={{ height: '100vh', width: '100vw', overflow: 'hidden', position: 'relative' }}>
      <div
        style={{
          position: 'absolute',
          top: 10,
          left: 10,
          zIndex: 10,
          padding: 10,
          backgroundColor: 'rgba(255,255,255,0.9)',
          borderRadius: 4,
          boxShadow: '0 2px 6px rgba(0,0,0,0.2)',
        }}
      >
        <div style={{ marginBottom: 8 }}>
          <input
            placeholder="Enter scramble (e.g. R U R' U')"
            value={scramble}
            onChange={(e) => setScramble(e.target.value)}
            disabled={isAnimating}
            style={{ width: 300, marginRight: 8, padding: 6, fontSize: 16 }}
          />
        </div>
        <div>
          <button 
            onClick={onScrambleClick} 
            disabled={isAnimating || !scramble}
            style={{ marginRight: 8 }}
          >
            {scrambling ? 'Scrambling...' : 'Apply Scramble'}
          </button>
          <button 
            onClick={onSolveClick} 
            disabled={isAnimating || !wasmModule.current || !scramble}
            style={{ marginRight: 8 }}
          >
            {solving ? 'Solving...' : 'Solve & Animate'}
          </button>
          <button 
            onClick={onResetClick} 
            disabled={isAnimating}
          >
            Reset Cube
          </button>
        </div>
      </div>

      <Canvas
        shadows
        camera={{ position: [5, 5, 5], fov: 50, near: 0.1, far: 1000 }}
        style={{ backgroundColor: '#171717' }}
      >
        <ambientLight intensity={0.6} />
        <directionalLight position={[10, 10, 10]} intensity={1} castShadow />
        <RubiksCube cubeState={cubeState} />
        <OrbitControls enablePan={false} />
      </Canvas>
    </div>
  );
}