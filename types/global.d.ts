export {}

declare global {
  interface Window {
    createSolverModule?: () => Promise<any>
  }
}
