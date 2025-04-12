# Project Ikarus by Hephaestus

**Project Ikarus by Hephaestus** is an advanced, modular AI companion designed specifically for survival multiplayer games, built on Unreal Engine 5.5. The AI companion leverages a robust Utility AI system that makes dynamic, context-aware decisions in real time. This project is my creative vision under the moniker *Hephaestus*, inspired by mythological craftsmanship and innovation.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Architecture & Roadmap](#architecture--roadmap)
- [Installation](#installation)
- [Usage](#usage)
- [Contributing](#contributing)
- [License](#license)
- [Contact](#contact)

## Overview

Project Ikarus by Hephaestus is designed to deliver a truly immersive AI companion experience for survival multiplayer games. The project incorporates multiple modules including:
- **Perception & Sensing:** Real-time detection of in-game stimuli.
- **Memory & Context:** Context-aware behavior based on past interactions and events.
- **Utility AI Decision-Making:** Dynamic choice selection using weighted utility functions.
- **Navigation & Locomotion:** Advanced pathfinding and responsive movement using UE5.5’s native features.
- **Communication & Interaction:** Integrated dialogue and command systems, tied with animation and audio feedback.

This open-source project is intended for developers seeking to integrate a sophisticated AI companion into their Unreal Engine projects or learn from its modular architecture.

## Features

- **Modular Architecture:** Each component (sensing, memory, decision-making, etc.) is developed as a standalone module with clear interfaces for ease of extension and maintenance.
- **Utility AI System:** Implements a dynamic utility-based decision system that adapts to environmental cues, enhancing realism and immersion.
- **Multiplayer Support:** Ensures state synchronization and performance optimization across networked multiplayer sessions.
- **Blueprint & C++ Hybrid:** Uses both Blueprints for rapid prototyping and C++ for performance-critical operations.
- **Extensible Framework:** Designed to allow easy addition of new behaviors, emotions, and AI functionalities.

## Architecture & Roadmap

For detailed architectural insights and future updates, please refer to the [ROADMAP.md](ROADMAP.md) file in this repository. Key modules include:

- **Perception & Sensing Module:** Handling environment scanning via line-of-sight, audio sensors, and proximity detection.
- **Memory & Context Module:** Storing and processing historical data to influence decision-making.
- **Utility AI Module:** Calculating action scores and integrating with Unreal Engine’s Behavior Trees.
- **Navigation & Locomotion Module:** Utilizing UE5.5's navigation systems for realistic movement and obstacle avoidance.
- **Communication & Interaction Module:** Managing dialogue systems, in-game feedback, and integration with player commands.

## Installation

### Prerequisites

- **Unreal Engine 5.5:** Ensure you have UE5.5 installed. You can download it from the [Epic Games Launcher](https://www.unrealengine.com/).
- **Git:** To clone and manage the repository.
- **C++ Compiler:** Compatible with Unreal Engine’s requirements.

### Cloning the Repository

Clone the repository using Git:

```bash
git clone https://github.com/your-username/Project-Ikarus-by-Hephaestus.git
