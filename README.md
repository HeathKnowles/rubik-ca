# 🧊 Rubik’s Cube Solver & 3D Visualizer

A high-performance, interactive Rubik’s Cube solver built with:

* 🧠 **C++**: Fast, memory-efficient solver compiled to WebAssembly
* 🎮 **React + Three.js**: Interactive 3D visualization of the cube in the browser
* 🌐 **Emscripten**: Bridge between C++ logic and JavaScript frontend

<br>

---

## 📸 Demo
 
 [Rubiks Solver](https://rubik-ca.vercel.app/)


---

## 🚀 Features

* 🔁 Apply scrambles like `"R U R' U'"` via input
* ⚙️ Real-time solver using a WebAssembly backend (IDA\* with pattern databases)
* 🎨 Smooth animated 3D cube with twist effects
* 🧮 Accurate cube state tracking and face rotations
* ♻️ Reset, scramble, solve — all interactively

---

## 🧰 Tech Stack

| Layer       | Technology                        |
| ----------- | --------------------------------- |
| Backend     | C++ (Rubik’s Cube IDA\* solver)   |
| WebAssembly | Emscripten                        |
| Frontend    | React, TypeScript                 |
| 3D Graphics | React Three Fiber, Drei, Three.js |
| UI          | HTML + CSS (custom styling)       |

---

## 🛠️ Setup Instructions

### 📦 1. Clone the Repository

```bash
git clone https://github.com/your-username/rubik-ca.git
cd rubiks-cube-wasm
```

### ⚙️ 2. Build the WebAssembly Solver (C++)

> Prerequisites: [Emscripten](https://emscripten.org/docs/getting_started/downloads.html)

```bash
emcc solver.cpp -O3 \
  -s WASM=1 \
  -s EXPORTED_FUNCTIONS="['_solveCube']" \
  -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap','UTF8ToString','stringToUTF8','lengthBytesUTF8','_malloc','_free']" \
  -s MODULARIZE=1 \
  -s 'EXPORT_NAME="SolverModule"' \
  -o public/solver.js
```

### 🌐 3. Install Frontend Dependencies

```bash
bun install
# or
npm install
# or
yarn install
```

### ▶️ 4. Start the Development Server

```bash
bun run dev
# or
npm run dev
# or
yarn dev
```

Then open [http://localhost:3000](http://localhost:3000) in your browser.

---

## 🧩 How It Works

### 📌 Backend: Cube Solver (C++)

* Represents cube state using edges, corners, and orientations
* Uses IDA\* search guided by:

  * Edge orientation
  * Corner orientation
  * E-slice position
* Solver is compiled to WebAssembly for high performance in-browser

### 🎥 Frontend: 3D Cube UI

* Uses React Three Fiber (`@react-three/fiber`) for real-time 3D rendering
* Each **cubie** is an independent mesh with correctly oriented stickers
* Applies scramble and solution moves via smooth animation
* Communicates with the WASM module using `malloc`, `_solveCube`, and `UTF8ToString`

---

## 🧪 Example Usage

1. Enter a scramble:

   ```
   Example:
   R U R' U'
   ```

2. Click **"Apply Scramble"** — the cube animates the moves.

3. Click **"Solve & Animate"** — the WASM solver returns a solution which gets applied visually.

4. Click **"Reset Cube"** to go back to solved state.

---

## 📄 License

MIT © 2025 Yash Yashuday

---