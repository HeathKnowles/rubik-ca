'use client'

import { Canvas } from '@react-three/fiber'
import { OrbitControls } from '@react-three/drei'
import { useMemo } from 'react'
import Cube from '@/components/cube'

export default function HomePage() {
  // Simulate a scrambled layout: slight randomized positions
  const cubePositions = useMemo(() => {
    return Array.from({ length: 3 }, (_, x) =>
      Array.from({ length: 3 }, (_, y) =>
        Array.from({ length: 3 }, (_, z) => {
          const pos: [number, number, number] = [x - 1, y - 1, z - 1]
          return {
            key: `${x}-${y}-${z}`,
            position: pos,
            rotation: [
              Math.floor(Math.random() * 4) * Math.PI / 2,
              Math.floor(Math.random() * 4) * Math.PI / 2,
              Math.floor(Math.random() * 4) * Math.PI / 2,
            ] as [number, number, number],
          }
        })
      )
    ).flat(2)
  }, [])

  return (
    <div className="h-screen w-full">
      <Canvas camera={{ position: [5, 5, 5] }}>
        <ambientLight intensity={0.5} />
        <directionalLight position={[3, 3, 3]} />
        <OrbitControls />
        <group>
          {cubePositions.map(({ key, position, rotation }) => (
            <Cube key={key} position={position} rotation={rotation} />
          ))}
        </group>
      </Canvas>
    </div>
  )
}
