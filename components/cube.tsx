'use client'

import { useRef } from 'react'
import { Mesh, MeshStandardMaterial } from 'three'

interface CubeProps {
  position: [number, number, number]
  rotation?: [number, number, number]
}

export default function Cube({ position, rotation }: CubeProps) {
  const meshRef = useRef<Mesh>(null)

  const materials = [
    new MeshStandardMaterial({ color: 'white' }),
    new MeshStandardMaterial({ color: '#FFDF22' }),
    new MeshStandardMaterial({ color: 'blue' }),
    new MeshStandardMaterial({ color: 'green' }),
    new MeshStandardMaterial({ color: 'red' }),
    new MeshStandardMaterial({ color: '#FF6F00' }), // better orange
  ]

  return (
    <mesh ref={meshRef} position={position} rotation={rotation} material={materials}>
      <boxGeometry args={[0.9, 0.9, 0.9]} />
    </mesh>
  )
}

