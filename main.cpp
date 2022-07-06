#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#include <ctype.h>
#include <cassert>

#include "chessLogic.cpp"
#include "chessEngine.cpp"


static const char cmd[] = "./stockfish/stockfish";

class Globals {
public:
  chessGame currentGame;
  engine engine1 = engine(cmd);
  engine engine2 = engine(cmd);

  int numPlayers = -1;
  bool whiteIsPlayer;
  bool blackIsPlayer;
  Globals(void){
    currentGame = chessGame();
  }
};

static Globals g;

#include "drawBoard.cpp"

void
restartGame(void)
{
  g.currentGame = chessGame();
  char newGameCmd[] = "ucinewgame\n";
  g.engine1.writeToEngine(newGameCmd);
  g.engine2.writeToEngine(newGameCmd);
}

void
handleWinConditions(void){
  std::vector<move> legalMoves = g.currentGame.generateLegalMoves(&g.currentGame.currentState);
  bool isInCheck = g.currentGame.isInCheck(&g.currentGame.currentState, g.currentGame.currentState.isWhitesTurn);
  if((legalMoves.size() == 0)){
    if(isInCheck){
      if(g.currentGame.currentState.isWhitesTurn){
	printf("checkmate -- black wins\n");
      }else{
	printf("checkmate -- white wins\n");
      }
    }else{
      printf("stalemate\n");
    }
    usleep(1000000);
    restartGame();
  }
  if(g.currentGame.currentState.halfMoves > 300){
    printf("stalemate -- too many moves without pawn advance or capture\n");
    usleep(1000000);
    restartGame();
  }
}

void
printGame(void){
  printf("================\n");
  printf("+");
  for(int i = 0; i < 8; i++){
    printf("---+");
  }
  printf("\n|");
  for(int y = 0; y < 8; y++){
    for(int x = 0; x < 8; x++){
      int index = x+(y*8);
      char piece = g.currentGame.currentState.board[index];
      if(piece == EMPTY){
	piece = ' ';
      }
      printf(" %c |", piece);
    }
    printf(" %d", 8-y);
    printf("\n+");
    for(int i = 0; i < 8; i++){
      printf("---+");
    }
    if(y != 7){
      printf("\n|");
    }else{
      printf("\n");
    }
  }
  printf("  ");
  for(int x = 0; x < 8; x++){
    printf("%c   ", 'a' + x);
  }
  printf("\n");
  printf("=================\n");
}

void
doEngineMove(engine* usethis){
  move engineMove = usethis->getBestMove(&g.currentGame.currentState);;
  bool success = g.currentGame.attemptMove(engineMove);
  assert(success);
}

void
doPlayerInput(void){
  printf("enter player command:\n");
  const int timeoutInfinite = -1;
  
  waitForData(fileno(stdin), timeoutInfinite);

  char buff[0xff];
  int numRead = read(fileno(stdin), buff, sizeof(buff)-1);
  buff[numRead] = '\0';
  
  char* cmdString;
  
  if((cmdString = strstr(buff, "move ")) != NULL){
    cmdString += sizeof("move ") - 1;
    char playerMove[5];
    
    char char1 = cmdString[0];
    char char2 = cmdString[1];
    char char3 = cmdString[2];
    char char4 = cmdString[3];
    char char5 = cmdString[4];
    
    char promotion = char5;
    if(!isalpha(promotion)){
      promotion = '\0';
    }
    playerMove[0] = char1;
    playerMove[1] = char2;
    playerMove[2] = char3;
    playerMove[3] = char4;
    playerMove[4] = promotion;
    if(!(isalpha(char1)&&isdigit(char2)&&isalpha(char3)&&isdigit(char4))){
      printf("move is not in format letter-digit-letter-digit\n");
      return;
    }
    
    printf("playermove \"%c%c%c%c%c\"\n", playerMove[0], playerMove[1], playerMove[2], playerMove[3], playerMove[4]);
    int fromX = playerMove[0]-'a';
    int fromY = 7 - (playerMove[1]-'1');
    int toX = playerMove[2]-'a';
    int toY = 7 - (playerMove[3]-'1');
	
    move output(fromX + (fromY*8), toX + (toY*8));
    if(isalpha(playerMove[4])){
      output.promotion = playerMove[4];
    }else{
      output.promotion = '\0';
    }
    bool success = g.currentGame.attemptMove(output);
    if(!success){
      printf("invalid move\n");
    }
    return;
  }
  if((cmdString = strstr(buff, "undo")) != NULL){
    g.currentGame.undo();
    g.currentGame.undo();
    return;
  }
  if((cmdString = strstr(buff, "redo")) != NULL){
    g.currentGame.redo();
    g.currentGame.redo();
    return;
  }
  printf("no command: \"%s\"\n", buff);
}

void
startGame(void){
  openGLDrawStuff();
  while(!glfwWindowShouldClose(mainWindow)){
    printGame();
    glfwLoopStuff();
    
    if(g.currentGame.currentState.isWhitesTurn){
      if(g.whiteIsPlayer){
	doPlayerInput();
      }else{
	doEngineMove(&g.engine1);
      }
    }else{
      if(g.blackIsPlayer){
	doPlayerInput();
      }else{
	doEngineMove(&g.engine2);
      }
    }
    
    openGLDrawStuff();
    handleWinConditions();
  }

  closeGLFW();
}

int
main(int argc, char* argv[])
{
  printf("Starting up============================\n");
  initGLStuff();
  setupOpenGLStuff();
  
  while(true){
    printf("How many players:\n");
    if(scanf ("%d", &g.numPlayers) != 1){
      printf("must be integer\n");
      continue;
    }
    printf("detected '%d' as input\n", g.numPlayers);
    if(g.numPlayers > 2){
      printf("must be <= 2\n");
    }else if(g.numPlayers < 0){
      printf("must be >= 0\n");
    }else{
      break;
    }
  }
  char playAsWhite = '\0';
  switch(g.numPlayers){
  case 0:
    g.whiteIsPlayer = false;
    g.blackIsPlayer = false;
    break;
  case 1:
    while(true){
      printf("play as while? [y/n]\n");
      if(scanf("%c", &playAsWhite) != 1){
	printf("answer with char\n");
      }
      printf("detected '%c' as input\n", playAsWhite);
      if((playAsWhite != 'y')&&(playAsWhite != 'n')){
	printf("please answer with 'y' or 'n'\n");
      }else{
	break;
      }
    }
    if(playAsWhite == 'y'){    
      g.whiteIsPlayer = true;
      g.blackIsPlayer = false;
    }else if(playAsWhite == 'n'){
      g.whiteIsPlayer = false;
      g.blackIsPlayer = true;
    }else{
      assert(false);
    }
    break;
  case 2:
    g.whiteIsPlayer = true;
    g.blackIsPlayer = true;
    break;
  default:
    assert(false);
  }
  
  generateDistanceToEdge();
  nodeTest(3, boardState());
  
  startGame();
  
  printf("All done============================\n");
  return 0;
}