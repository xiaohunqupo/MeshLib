[![build-test-distribute](https://github.com/MeshInspector/MeshLib/actions/workflows/build-test-distribute.yml/badge.svg?branch=master)](https://github.com/MeshInspector/MeshLib/actions/workflows/build-test-distribute.yml?branch=master) 

[![PyPI version](https://badge.fury.io/py/meshlib.svg)](https://badge.fury.io/py/meshlib)
[![Downloads](https://pepy.tech/badge/meshlib/month?style=flat-square)](https://pepy.tech/project/meshlib)
[![Python](https://img.shields.io/pypi/pyversions/meshlib.svg?style=flat)](https://badge.fury.io/py/meshlib)

[![NuGet Version](https://img.shields.io/nuget/v/MeshLib?label=nuget%20package&color=green)](https://www.nuget.org/packages/MeshLib)
[![NuGet Downloads](https://img.shields.io/nuget/dt/MeshLib?label=downloads%2Ftotal&color=blue)](https://www.nuget.org/packages/MeshLib)

<p align="left">
<picture>
  <source media="(prefers-color-scheme: dark)"  srcset="https://github.com/user-attachments/assets/37d3a562-581d-421b-8209-ef6b224e96a8">
  <source media="(prefers-color-scheme: light)" srcset="https://github.com/user-attachments/assets/caf6bdd1-b2f1-4d6d-9e22-c213db6fc9cf">
  <img alt="Shows a black logo in light color mode and a white one in dark color mode." src="https://github.com/user-attachments/assets/caf6bdd1-b2f1-4d6d-9e22-c213db6fc9cf" width="60%">
</picture>
</p>

# MeshLib: An SDK to Supercharge Your 3D Data Processing Efficiency

The MeshLib SDK is an open-source library that provides advanced algorithms for 3D data processing. It assists developers and engineers in achieving precise results while delivering significant resource savings. Whether you are working on 3D printing, scanning, inspection, automation, robotics, scientific visualization, or medical devices, MeshLib is ideally suited to meet your needs thanks to its potent capabilities and broad applicability.

![MeshLib_SDK_Mesh_Processing_Library](https://github.com/user-attachments/assets/3be4bc4e-7657-4ba0-9246-17182c3ea5e6)

## Why Choose MeshLib

**Fully Open Source.** MeshLib is open to your ideas—help the team review, improve, or extend the library to better suit your project needs.  You can also fork the code for your own unique use cases.

**Multi-Language Support.** Written in C++ with bindings for C, C#, and Python, MeshLib integrates easily into AI pipelines and cross-platform workflows.

**High Performance.** Internal benchmarks show up to 10x faster execution compared to alternative SDKs—especially in 3D boolean operations. [See performance data](https://meshlib.io/blog/).

**GPU-Accelerated Architecture.** Built with speed and scalability in mind, MeshLib supports GPU acceleration and CUDA for high-performance computing.

**Cross-Platform Ready.** MeshLib runs on Windows, macOS, Linux, and WebAssembly, offering flexibility for any development environment.

**Developer Support.** Get timely assistance from our responsive support team for integration, usage, or troubleshooting.

**Flexible Integration.** Use MeshLib as a standalone engine with UI components, or integrate selected algorithms into existing applications with minimal dependencies.

## Wrapping Up

MeshLib provides a robust foundation for 3D data processing, supporting all essential formats like point clouds, meshes, and volumes continuously generated by modern sensors. Our powerful half-edge data structure ensures manifold compliance for precise, reliable mesh representation. Plus, our repository includes clear code samples to help you get started quickly and explore advanced features with ease.

## Key Available Algorithms with MeshLib

- **3D Boolean** performs fast, highly optimized mesh- and voxel-based operations.
- **Mesh Repair** eliminates self-intersections, fills holes, and removes degeneracies.
- **Mesh Offsetting** controls surface thickness with multiple precise modes for 3D printing and machining.
- **Hole Filling** fills flat and curved surfaces, connects or separates holes, and builds bridges.
- **Mesh Simplification** optimizes mesh complexity while keeping details within set tolerance. We provide remeshing, and subdivision options as well.
- **Collision Detection** verifies intersections between models for further operations.
- **Extensive File Format Support** enables importing a wide range of file formats for meshes, point clouds, CT scans, polylines, distance maps, and G-code. Export functionalities—and support for color and texture data—are available for select formats, too (see the [full list for details](https://meshlib.io/feature/file-formats-supported-by-meshlib/)).
- **Triangulation** converts point clouds into meshes with accurate normal creation.
- **ICP** precisely aligns meshes using point-to-point and point-to-plane transformations.
- **Segmentation** performs semi-automatic segmentation based on curvature for meshes and voxels.
- **Deformation** applies Laplacian, freeform, and relaxation smoothing for fine mesh adjustments.
- **Support of Distance Maps and Polylines** allows to generate distance maps and iso-lines and performs projection and intersection.

For detailed information, explore our [MeshLib website section](https://meshlib.io/features/) or refer to the corresponding sections in our [documentation](https://meshlib.io/documentation/index.html).

## How to Get Started with MeshLib

- **Evaluate MeshLib.** Start by exploring MeshLib for free under our [educational and non-commercial license](https://github.com/MeshInspector/MeshLib?tab=License-1-ov-file#readme). You’ll get access to the [documentation](https://meshlib.io/documentation/index.html), installation guide, example code, and can run scripts locally to see how it fits your workflow.
- **Try MeshInspector.** Put MeshLib to the test using [MeshInspector](https://meshinspector.com/), our GUI built on top of the SDK. It's available as a standalone desktop and web app with a 30-day trial.
- [**Request a Demo**](https://meshlib.io/book-a-call/). Get expert-level guidance, ask questions about integration, and receive complete licensing information tailored to your needs.
- **Visit our blog.** Explore [articles and tutorials](https://meshlib.io/blog/) covering 3D data processing workflows, occasional comparisons with other tools, and practical insights from the field.

## Installation

For Python, simply install via pip:

```
pip install meshlib
```

If your chose is C++, C or C#, check out our [Installation Guide](https://meshlib.io/documentation/InstallationGuide.html).

Here, you can find [MeshLib's tutorials](https://meshlib.io/documentation/Tutorials.html) and [code samples](https://meshlib.io/documentation/Examples.html) to master our SDK quickly and with ease.

## **License**

Here, you can access our [Non-Commercial Free License](https://github.com/MeshInspector/MeshLib?tab=License-1-ov-file#readme) with a Commercial License Requirement. Also, see extra details on [MeshLib license page](https://meshlib.io/license/).

## **Reporting**

Report bugs via our [GitHub Issues Form](https://www.notion.so/Synchronize-messaging-on-the-website-and-on-github-readme-1ba37b78f273802bae15c430c8e14594?pvs=21) for efficient tracking and resolution. 

Join the [GitHub Discussions](https://www.notion.so/Synchronize-messaging-on-the-website-and-on-github-readme-1ba37b78f273802bae15c430c8e14594?pvs=21) to connect with developers, share ideas, and stay updated on MeshLib.
