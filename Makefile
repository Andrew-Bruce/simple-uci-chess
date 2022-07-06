compiler = g++
flags := -Wall -lglfw -lGLEW -lGL

srcs := main.cpp
srcs += chessEngine.cpp
srcs += chessLogic.cpp
srcs += drawBoard.cpp

foo:	$(srcs)
	$(compiler) $(flags) $< -o $(@)

clean:
	rm *~
