# chess
This is the Chess game, written in C using SDL for fun as a university project.

# How to compile using Visual Studio Code:

> 1) Open the project folder in **_VS Code_**
> 2) Make sure you installed LLVM for clang. Oherwise [Download Clang Here](https://releases.llvm.org/download.html)
> 3) Include paths may not be found until you open a random .c file and wait for the path's scan finishes.
> 4) Hit **CTRL + SHIFT + B** to compile it, the task will be called and the output will be generated inside **bin/** folder.
> 5) F5 to launch with debugger or feel free to run it from it's folder!

# Features:
- Castling: Supported Castling from both sides (long && short Castling)
- Enpassant: Supported.
- Pawn promotion: Supported: everytime a pawn reaches the opposite side of the board, you can choose to promote your pawn.

# Notes:
Since i wrote this game from scratch without implementing any kind of special graph search algorithm, The king's Checkmate algorithm may not work properly in some situation that i couldn't test.

Have Fun!

----------------------------------------------------------------------------

<p float="left">
  <img src="https://user-images.githubusercontent.com/7602472/162612643-bac28397-7bf5-43d5-a133-b2621cefb27e.png" width=40% height=40%>
  <img src="https://user-images.githubusercontent.com/7602472/162613034-e29f3e65-31de-489a-be6e-532cb98ec55c.png" width=40% height=40%>
</p>
