# Kuber3D

Kuber3D is a lightweight interactive 3D geometry sandbox designed for learning and demonstrating stereometry.  
It allows you to create and edit simple 3D figures using points, edges, and faces, inspect constructions from any camera angle, and perform basic geometric operations (angles, splitting, perpendiculars, etc.).

Built with **C++**, **OpenGL**, **GLFW**, **GLM**, and **Dear ImGui**.

---

## Features

- **3D viewport with orbit camera**
  - Smooth camera targeting and rotation
  - Grid-based orientation

- **Point-based construction**
  - Add points and connect them with lines
  - Create faces/quads for solid visualization
  - Selection & edit workflow

- **Editing & operations (keyboard-driven)**
  - Mouse picking / selection
  - Connect / disconnect points with lines
  - Split a segment by ratio input
  - Create a point on a plane (plane defined by 3 points)
  - Build perpendicular from a point to a plane

- **Angle measurement**
  - Create angle objects from 3 selected points
  - Arc visualization

- **Rendering**
  - Solid faces + wireframe lines
  - Dashed hidden lines (depth-based rendering)
  - Overlay UI (ImGui)

- **Built-in presets**
  - Cube
  - Pyramid
  - Tetrahedron
  - Circle (WIP / experimental)

---

## Controls

- `TAB` — toggle **Edit Mode**
- `LMB drag` — rotate camera
- `LMB click` (Edit Mode) — select points  
  - `SHIFT` — multi-select
- `DELETE` — delete selected points
- `J` — connect 2 selected points with a line
- `CTRL + J` — disconnect/remove a line between 2 selected points
- `S` — split a segment (select 2 points, then enter ratio `a` and `b`)
- `P` — create a point on a plane (select 3 points that define a plane)
- `R` — build perpendicular from a point to a plane  
  - select **1 point outside the plane** + **3 points on the plane**
- `V` — set camera target to the selected point
- `G` — create line in a face using intersection logic (advanced tool / WIP)
- `A` — create an angle (select 3 points)

---

## How it works (high level)

Kuber3D stores geometry as dynamic buffers (points, colors, indices).  
User interaction is based on screen-space picking: 3D points are projected to screen coordinates, and the nearest point within a pixel radius is selected. Editing operations modify the geometry data and rebuild GPU buffers when needed.

---

## Build / Run

Build steps depend on your environment/toolchain (Visual Studio / MinGW / etc.).  
The project uses OpenGL + GLFW and includes an ImGui-based UI layer.

Typical dependencies:

- OpenGL
- GLFW
- GLM
- Dear ImGui

---

## Roadmap / Ideas

- Better labeling system (A, B, C, D, A1, B1, C1, D1)
- Robust figure construction tools (prisms, pyramids, parallelepipeds)
- Measurements (lengths, angles, distances to plane/line)
- Save/load scenes
- Improved UX: gizmos, snapping, constraints