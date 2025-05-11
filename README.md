# 1v1 Chess Game – C++ OOP with SFML

**National University of Computer and Emerging Sciences (FAST-NUCES)**  
**Instructor:** Talha Shahid  
**Submission Date:** 11 May 2025  

## Group Members
- Muhammad Adil Saeed – 24K-0705  
- Zaid Faraz – 24K-0636  
- Muhammad Haseem Sammo – 24K-0666  

---

## 📝 Summary

### Overview
This project is a fully functional 1v1 chess game built in C++ using Object-Oriented Programming (OOP) principles and SFML for graphics, audio, and timing. It features turn-based play, audio cues, and file handling for saving game data.

### Key Features
- Complete implementation of OOP principles  
- Real-time timers and turn-based audio using SFML  
- File handling for saving and reloading game data  

---

## 🎯 Introduction

### Background
Chess presents a rich set of rules and edge cases, making it ideal for practicing OOP concepts such as encapsulation, inheritance, and polymorphism.

### Objectives
- Implement a rule-compliant chess game in C++ using OOP  
- Add turn-based timer functionality  
- Enable saving and loading of game data  

---

## 📌 Project Description

### Scope

**Inclusions:**
- Standard chess rules including:
  - Castling
  - En passant
  - Pawn promotion
- Real-time timers using SFML
- Audio notifications per turn
- File-based save/load mechanism
- (Optional) Undo move feature

**Exclusions:**
- No AI opponent
- No online multiplayer

### Technical Overview
- **Language:** C++  
- **Library:** SFML (Graphics, Audio, Timing)  
- **IDE:** Visual Studio Code  
- **File Handling:** Standard C++ I/O  

---

## 🛠️ Methodology

### Approach
The team followed a sprint-based development cycle. Weekly tasks were divided and then integrated through peer review and testing.

### Roles and Responsibilities
- **Muhammad Adil Saeed:** Timer logic with SFML  
- **Muhammad Haseem Sammo:** File handling, reporting, and documentation  
- **Zaid Faraz Syed:** Core architecture and game logic lead  

---

## 🔧 Project Implementation

### Design and Structure

**Class Hierarchy:**
- `ChessPiece` (Base Class)  
  - `King`, `Queen`, `Bishop`, `Knight`, `Rook`, `Pawn` (Derived Classes)  
- Supporting Classes:
  - `ChessBoard`, `ChessGame`

**OOP Concepts Used:**
- **Encapsulation:** Grouping related data and behaviors  
- **Inheritance:** All pieces inherit from a common base class  
- **Polymorphism:** Virtual functions for move validation, rendering  

### Functionalities
- Complete 2-player rule-based chess  
- Turn timer using SFML clock  
- File-based game state saving and loading  

### Challenges
- Implementing 2D graphical interface  
- Handling edge rules like checkmate and castling  
- Maintaining consistent file structure for game state  

---

## ✅ Results

### Outcomes
- Functional 1v1 chess game with:
  - SFML-based timers
  - Turn audio cues
  - Game history persistence
- Demonstrates strong use of OOP principles in C++

### Testing and Validation
- Manual testing for illegal moves and game-ending conditions  
- Timer validated using SFML  
- Save/load tested across multiple game sessions  

---

## 📌 Conclusion

### Summary
The project successfully demonstrates encapsulation, inheritance, and polymorphism in a real-world game scenario. It also integrates graphical, timing, and file handling features effectively.

### Final Remarks
The system serves as a strong baseline for further features like AI integration and networked multiplayer.

---

## 🔮 Future Improvements

- Add AI-based single-player mode  
- Implement online multiplayer  
- Include a main menu and game settings  

---

## 📚 References

- [cppreference.com – C++ Documentation](https://en.cppreference.com/)  
- [GeeksforGeeks – OOP & Chess](https://www.geeksforgeeks.org/)  
- [SFML Documentation](https://www.sfml-dev.org/documentation/2.5.1/)  
- [YouTube – Hopzbie](https://www.youtube.com/@hopzbie)

