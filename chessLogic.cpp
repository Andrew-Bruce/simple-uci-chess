#include <vector>

typedef unsigned char UInt8;

enum {
  BP = 'p',
  BR = 'r',
  BN = 'n',
  BB = 'b',
  BQ = 'q',
  BK = 'k',
  
  WP = 'P',
  WR = 'R',
  WN = 'N',
  WB = 'B',
  WK = 'K',
  WQ = 'Q',

  EMPTY = '\0'
};

#define __ EMPTY
const UInt8 startingBoard[64] = {
				  BR, BN, BB, BQ, BK, BB, BN, BR,
				  BP, BP, BP, BP, BP, BP, BP, BP,
				  __, __, __, __, __, __, __, __,
				  __, __, __, __, __, __, __, __,
				  __, __, __, __, __, __, __, __,
				  __, __, __, __, __, __, __, __,
				  WP, WP, WP, WP, WP, WP, WP, WP,
				  WR, WN, WB, WQ, WK, WB, WN, WR
};
#undef __

const int BoxSize = 150;

const int dirLeft = -1;
const int dirRight = 1;
const int dirUp = -8;
const int dirDown = 8;
const int dirUpLeft = dirUp+dirLeft;
const int dirUpRight = dirUp+dirRight;
const int dirDownLeft = dirDown+dirLeft;
const int dirDownRight = dirDown+dirRight;

const int directions[8] = {dirLeft, dirRight, dirUp, dirDown, dirUpLeft, dirUpRight, dirDownLeft, dirDownRight};

const int actualDirections[8][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {1, -1}, {-1, 1}, {1, 1}};

int numSquaresTillEdge[64][8];


class move
{
public:
  int from;
  int to;
  char promotion;
  
  move(int from_, int to_){
    from = from_;
    to = to_;
    promotion = '\0';
  }
};


class boardState
{
public:
  UInt8 board[64];
  bool isWhitesTurn;
  bool whiteCanCastleQueenSide;
  bool whiteCanCastleKingSide;
  bool blackCanCastleQueenSide;
  bool blackCanCastleKingSide;
  int  enPassantPos;
  int halfMoves;
  int fullMoves;

  boardState(){
    memcpy(&board, &startingBoard, sizeof(startingBoard));
    isWhitesTurn = true;
    whiteCanCastleQueenSide = true;
    whiteCanCastleKingSide = true;
    blackCanCastleQueenSide = true;
    blackCanCastleKingSide = true;
    enPassantPos = -1;
    halfMoves = 0;
    fullMoves = 1;
  }

  bool
  isEmpty(int pos){
    if(!((pos >= 0)&&(pos < 64))){
      return false;
    }
    return (board[pos] == EMPTY);
  }

  bool
  isPawn(int pos){
    if(!((pos >= 0)&&(pos < 64))){
      return false;
    }
    return (board[pos] == BP)||(board[pos] == WP);
  }

  bool
  isRook(int pos){
    if(!((pos >= 0)&&(pos < 64))){
      return false;
    }
    return (board[pos] == BR)||(board[pos] == WR);
  }

  bool
  isKnight(int pos){
    if(!((pos >= 0)&&(pos < 64))){
      return false;
    }
    return (board[pos] == BN)||(board[pos] == WN);
  }

  bool
  isBishop(int pos){
    if(!((pos >= 0)&&(pos < 64))){
      return false;
    }
    return (board[pos] == BB)||(board[pos] == WB);
  }

  bool
  isQueen(int pos){
    if(!((pos >= 0)&&(pos < 64))){
      return false;
    }
    return (board[pos] == BQ)||(board[pos] == WQ);
  }

  bool
  isKing(int pos){
    if(!((pos >= 0)&&(pos < 64))){
      return false;
    }
    return (board[pos] == BK)||(board[pos] == WK);
  }

  bool
  isBlack(int pos){
    if(!((pos >= 0)&&(pos < 64))){
      return false;
    }
    if(board[pos] == EMPTY){
      return false;
    }
    return islower(board[pos]);
  }

  bool
  isWhite(int pos){
    if(!((pos >= 0)&&(pos < 64))){
      return false;
    }
    if(board[pos] == EMPTY){
      return false;
    }
    return isupper(board[pos]);
  }

  bool
  isAttackable(int pos){
    if(isEmpty(pos)){
      return true;
    }
    if(isWhitesTurn){
      return isBlack(pos);
    }else{
      return isWhite(pos);
    }
  }
  
  char*
  convertBoardToFen(void)
  {
    int bufferSize = 128;
    char* boardFen = (char*)malloc(bufferSize*sizeof(char));
    memset(boardFen, 0, bufferSize*sizeof(char));
    char *p = boardFen;
    for(int y = 0; y < 8; y++) {
      int consecutiveBlanks = 0;
      for(int x = 0; x < 8; x++) {
	UInt8 piece = board[(y*8) +x];
	if (piece == EMPTY) {
	  ++consecutiveBlanks;
	} else {
	  if (consecutiveBlanks != 0) {
	    *p++ = '0' + consecutiveBlanks;
	    consecutiveBlanks = 0;
	  }
	  *p++ = piece;
	}
      }
      if (consecutiveBlanks != 0) {
	*p++ = '0' + consecutiveBlanks;
      }
      if (y < 8-1) {
	*p++ = '/';
      }
    }
    *p++ = ' ';
    *p++ = isWhitesTurn ? 'w' : 'b';
    *p++ = ' ';
    if (whiteCanCastleQueenSide ||
	whiteCanCastleKingSide ||
	blackCanCastleQueenSide ||
	blackCanCastleKingSide) {
      if (whiteCanCastleKingSide) { *p++ = 'K'; }
      if (whiteCanCastleQueenSide)  { *p++ = 'Q'; }
      if (blackCanCastleKingSide) { *p++ = 'k'; }
      if (blackCanCastleQueenSide)  { *p++ = 'q'; }
    } else {
      *p++ = '-';
    }
    *p++ = ' ';
    if(enPassantPos != -1){
      *p++ = 'a' + (enPassantPos%8);
      *p++ = '8' - (enPassantPos/8);
    }else{
      *p++ = '-';
    }
    
    sprintf(p, " %d %d", halfMoves, fullMoves);
    p += strlen(p);
    assert(*p == 0);
    assert(strlen(boardFen) < (bufferSize*sizeof(char) + 10));

    return boardFen;
  }
};

class chessGame
{
public:
  boardState currentState;

  static const int maxBackup = 300;
  boardState backupInfo[maxBackup];
  int farthestBackup = 0;
  int currentBackup = 0;

  chessGame(void){
    backupInfo[0] = boardState();
  }
  void
  undo(){
    if(currentBackup-1 >= 0){
      currentBackup--;
      currentState = backupInfo[currentBackup];
    }else{
      printf("cannot undo to before game start\n");
    }
  }
  
  void
  redo(){
    if(currentBackup+1 <= farthestBackup){
      currentBackup++;
      currentState = backupInfo[currentBackup];
    }else{
      printf("farthest redo reached\n");
    }
  }
  
  void
  backupCurrentState(void){
    if(currentBackup+1 < maxBackup){
      currentBackup++;
      farthestBackup = currentBackup;
      backupInfo[farthestBackup] = currentState;
    }
  }
  
  static void
  forceMove(move forcedMove, boardState* state){
    int fromPos = forcedMove.from;
    int toPos = forcedMove.to;
    
    int fromX = fromPos%8;
    int fromY = fromPos/8;
    int toX = toPos%8;
    int toY = toPos/8;
    
    
    
    if((state->isPawn(fromPos))||(!state->isEmpty(toPos))){
      state->halfMoves = 0;
    }else{
      state->halfMoves++;
    }
      
    if(!state->isWhitesTurn){
      state->fullMoves++;
    }
    
    if(state->isKing(fromPos)){
      if(state->isBlack(fromPos)){
	state->blackCanCastleQueenSide = false;
	state->blackCanCastleKingSide = false;
	if((fromX == 4)&&(fromY == 0)){
	  if(toY == 0){
	    if(toX == 2){
	      state->board[0] = EMPTY;
	      state->board[3] = BR;
	    }
	    if(toX == 6){
	      state->board[7] = EMPTY;
	      state->board[5] = BR;
	    }
	  }
	}
      }
      if(state->isWhite(fromPos)){
	state->whiteCanCastleQueenSide = false;
	state->whiteCanCastleKingSide = false;
	if((fromX == 4)&&(fromY == 7)){
	  if(toY == 7){
	    if(toX == 2){
	      state->board[56] = EMPTY;
	      state->board[59] = WR;
	    }
	    if(toX == 6){
	      state->board[63] = EMPTY;
	      state->board[61] = WR;
	    }
	  }
	}
      }
    }

    if(state->isRook(fromPos)){
      if(state->isBlack(fromPos)){
	if((fromX == 0)&&(fromY == 0)){
	  state->blackCanCastleQueenSide = false;
	}
	if((fromX == 7)&&(fromY == 0)){
	  state->blackCanCastleKingSide = false;
	}
      }
      if(state->isWhite(fromPos)){
	if((fromX == 0)&&(fromY == 7)){
	  state->whiteCanCastleQueenSide = false;
	}
	if((fromX == 7)&&(fromY == 7)){
	  state->whiteCanCastleKingSide = false;
	}
      }
    }
    if((toX == 0)&&(toY == 0)){
      state->blackCanCastleQueenSide = false;
    }
    if((toX == 7)&&(toY == 0)){
      state->blackCanCastleKingSide = false;
    }
    if((toX == 0)&&(toY == 7)){
      state->whiteCanCastleQueenSide = false;
    }
    if((toX == 7)&&(toY == 7)){
      state->whiteCanCastleKingSide = false;
    }
    
    if(state->isPawn(fromPos)){
      if(toPos == state->enPassantPos){
	if(state->isWhite(fromPos)){
	  state->board[toPos+dirDown] = EMPTY;
	}
	if(state->isBlack(fromPos)){
	  state->board[toPos+dirUp] = EMPTY;
	}
      }
    }
    state->enPassantPos = -1;
    if(state->isPawn(fromPos)){
      if(abs(fromY-toY) == 2){
	if(state->isWhite(fromPos)){
	  state->enPassantPos = fromPos+dirUp;
	}
	if(state->isBlack(fromPos)){
	  state->enPassantPos = fromPos+dirDown;
	}
      }
    }
    
    state->isWhitesTurn = !state->isWhitesTurn;


    char promotion = forcedMove.promotion;
    if(state->isPawn(fromPos)){
      if(state->isWhite(fromPos)){
	if(toY == 0){
	  if(promotion == 'q'){
	    state->board[toPos] = 'Q';
	  }else if(promotion == 'r'){
	    state->board[toPos] = 'R';
	  }else if(promotion == 'b'){
	    state->board[toPos] = 'B';
	  }else if(promotion == 'n'){
	    state->board[toPos] = 'N';
	  }else{
	    printf("WHITE promotion is something wierd: '%c'\n", promotion);
	    assert(false);
	  }
	  state->board[fromPos] = EMPTY;
	  return;
	}
      }
      if(state->isBlack(fromPos)){
	if(toY == 7){
	  if(promotion == 'q'){
	    state->board[toPos] = 'q';
	  }else if(promotion == 'r'){
	    state->board[toPos] = 'r';
	  }else if(promotion == 'b'){
	    state->board[toPos] = 'b';
	  }else if(promotion == 'n'){
	    state->board[toPos] = 'n';
	  }else{
	    printf("BLACK promotion is something wierd: '%c'\n", promotion);
	    assert(false);
	  }
	  state->board[fromPos] = EMPTY;
	  return;
	}
      }
    }
    state->board[toPos] = state->board[fromPos];
    state->board[fromPos] = EMPTY;
    
  }

  static std::vector<move>
  generatePseudoLegalMoves(boardState* state){
    std::vector<move> moveList;
    for(int i = 0; i < 64; i++){
      if((state->isWhitesTurn&&state->isWhite(i))||((!state->isWhitesTurn)&&state->isBlack(i))){
	if(state->isPawn(i)){
	  int fromY = i/8;
	  int fromX = i%8;
	  if(state->isWhitesTurn){
	    if(state->isEmpty(i+dirUp)){
	      int toY = (i+dirUp)/8;
	      if(toY == 0){
		move temp = move(i, i+dirUp);
		temp.promotion = 'q';
		moveList.push_back(temp);
		temp.promotion = 'r';
		moveList.push_back(temp);
		temp.promotion = 'b';
		moveList.push_back(temp);
		temp.promotion = 'n';
		moveList.push_back(temp);
	      }else{
		moveList.push_back(move(i, i+dirUp));
	      }
	    }
	    if(fromX != 7){
	      if((state->isBlack(i+dirUp+dirRight))||(state->enPassantPos == i+dirUp+dirRight)){
		int toY = (i+dirUp+dirRight)/8;
		if(toY == 0){
		  move temp = move(i, i+dirUp+dirRight);
		  temp.promotion = 'q';
		  moveList.push_back(temp);
		  temp.promotion = 'r';
		  moveList.push_back(temp);
		  temp.promotion = 'b';
		  moveList.push_back(temp);
		  temp.promotion = 'n';
		  moveList.push_back(temp);
		}else{
		  moveList.push_back(move(i, i+dirUp+dirRight));
		}
	      }
	    }
	    if(fromX != 0){
	      if((state->isBlack(i+dirUp+dirLeft))||(state->enPassantPos == i+dirUp+dirLeft)){
		int toY = (i+dirUp+dirLeft)/8;
		if(toY == 0){
		  move temp = move(i, i+dirUp+dirLeft);
		  temp.promotion = 'q';
		  moveList.push_back(temp);
		  temp.promotion = 'r';
		  moveList.push_back(temp);
		  temp.promotion = 'b';
		  moveList.push_back(temp);
		  temp.promotion = 'n';
		  moveList.push_back(temp);
		}else{
		  moveList.push_back(move(i, i+dirUp+dirLeft));
		}
	      }
	    }
	    if(fromY == 6){
	      if((state->isEmpty(i+dirUp))&&(state->isEmpty(i+2*dirUp))){
		moveList.push_back(move(i, i+2*dirUp));
	      }
	    }
	  }else{
	    if(state->isEmpty(i+dirDown)){
	      int toY = (i+dirDown)/8;
	      if(toY == 7){
		move temp = move(i, i+dirDown);
		temp.promotion = 'q';
		moveList.push_back(temp);
		temp.promotion = 'r';
		moveList.push_back(temp);
		temp.promotion = 'b';
		moveList.push_back(temp);
		temp.promotion = 'n';
		moveList.push_back(temp);
	      }else{
		moveList.push_back(move(i, i+dirDown));
	      }
	    }
	    if(fromX != 7){
	      if((state->isWhite(i+dirDown+dirRight))||(state->enPassantPos == i+dirDown+dirRight)){
		int toY = (i+dirDown+dirRight)/8;
		if(toY == 7){
		  move temp = move(i, i+dirDown+dirRight);
		  temp.promotion = 'q';
		  moveList.push_back(temp);
		  temp.promotion = 'r';
		  moveList.push_back(temp);
		  temp.promotion = 'b';
		  moveList.push_back(temp);
		  temp.promotion = 'n';
		  moveList.push_back(temp);
		}else{
		  moveList.push_back(move(i, i+dirDown+dirRight));
		}
	      }
	    }
	    if(fromX != 0){
	      if((state->isWhite(i+dirDown+dirLeft))||(state->enPassantPos == i+dirDown+dirLeft)){
		int toY = (i+dirDown+dirLeft)/8;
		if(toY == 7){
		  move temp = move(i, i+dirDown+dirLeft);
		  temp.promotion = 'q';
		  moveList.push_back(temp);
		  temp.promotion = 'r';
		  moveList.push_back(temp);
		  temp.promotion = 'b';
		  moveList.push_back(temp);
		  temp.promotion = 'n';
		  moveList.push_back(temp);
		}else{
		  moveList.push_back(move(i, i+dirDown+dirLeft));
		}
	      }
	    }
	    if(fromY == 1){
	      if((state->isEmpty(i+dirDown))&&(state->isEmpty(i+2*dirDown))){
		moveList.push_back(move(i, i+2*dirDown));
	      }
	    }
	  }
	  continue;
	}
	
	if(state->isKing(i)){
	  if(state->isWhitesTurn){
	    if(state->whiteCanCastleQueenSide){
	      if((state->isEmpty(i+dirLeft))&&(state->isEmpty(i+2*dirLeft))&&(state->isEmpty(i+3*dirLeft))){
		moveList.push_back(move(i, i+2*dirLeft));
	      }
	    }
	    if(state->whiteCanCastleKingSide){
	      if((state->isEmpty(i+dirRight))&&(state->isEmpty(i+2*dirRight))){
		moveList.push_back(move(i, i+2*dirRight));
	      }
	    }
	  }else{
	    if(state->blackCanCastleQueenSide){
	      if((state->isEmpty(i+dirLeft))&&(state->isEmpty(i+2*dirLeft))&&(state->isEmpty(i+3*dirLeft))){
		moveList.push_back(move(i, i+2*dirLeft));
	      }
	    }
	    if(state->blackCanCastleKingSide){
	      if((state->isEmpty(i+dirRight))&&(state->isEmpty(i+2*dirRight))){
		moveList.push_back(move(i, i+2*dirRight));
	      }
	    }
	  }

	  for(int dir = 0; dir < 8; dir++){
	    if(state->isAttackable(i+directions[dir])){
	      if(numSquaresTillEdge[i][dir] > 1){
		moveList.push_back(move(i, i+directions[dir]));
	      }
	    }
	  }
	  continue;
	}
	
	if(state->isKnight(i)){
	  for(int dx = -1; dx <= 1; dx += 2){
	    for(int dy = -1; dy <= 1; dy += 2){
	      int fx;
	      int fy;
	      
	      fx = (i%8)+(1*dx);
	      fy = (i/8)+(2*dy);
	      if(!((fx < 0)||(fx >= 8))){
		if(state->isAttackable((fy*8)+fx)){
		  moveList.push_back(move(i, (fy*8)+fx));
		}
	      }
	      
	      fx = (i%8)+(2*dx);
	      fy = (i/8)+(1*dy);
	      if(!((fx < 0)||(fx >= 8))){
		if(state->isAttackable((fy*8)+fx)){
		  moveList.push_back(move(i, (fy*8)+fx));
		}
	      }
	      
	    }
	  }
	  continue;
	}
	

	if(true){//all other pieces, aka slidey ones
	  int startDir = 0;
	  int endDir = 8;
	  if(state->isBishop(i)){
	    startDir = 4;
	  }
	  if(state->isRook(i)){
	    endDir = 4;
	  }
	  for(int dir = startDir; dir < endDir; dir++){
	    for(int s = 1; s < numSquaresTillEdge[i][dir]; s++){
	      if(state->isAttackable(i+s*directions[dir])){
		moveList.push_back(move(i, i+s*directions[dir]));
		if(!(state->isEmpty(i+s*directions[dir]))){
		  break;
		}
	      }else{
		break;
	      }
	    }
	  }
	}
      }
    }
    return moveList;
  }
  

  static bool
  isInCheck(boardState* state, bool isWhite){
    bool tmp = state->isWhitesTurn;
    state->isWhitesTurn = !isWhite;
    std::vector<move> moves = generatePseudoLegalMoves(state);
    state->isWhitesTurn = tmp;
    int kingPos = -1;
    for(int i = 0; i < 64; i++){
      if(state->isKing(i)){
	if(((isWhite)&&(state->isWhite(i)))||((!isWhite)&&(state->isBlack(i)))){
	  kingPos = i;
	  break;
	}
      }
    }
    if(kingPos == -1){
      printf("NO KING ON BOARD???\n");
      assert(false);
    }
    for(int i = 0; i < (int)moves.size(); i++){
      if(moves[i].to == kingPos){
	return true;
      }
    }
    return false;
  }
  
  static std::vector<move>
  generateLegalMoves(boardState* state){
    std::vector<move> pseudoLegals = generatePseudoLegalMoves(state);

    std::vector<move> legalMoves;
    
    bool isWhite = state->isWhitesTurn;
    for(int i = 0; i < (int)pseudoLegals.size(); i++){
      boardState tmp = *state;
      forceMove(pseudoLegals[i], &tmp);
      if(!isInCheck(&tmp, isWhite)){
	legalMoves.push_back(pseudoLegals[i]);
      }
    }
    
    return legalMoves;
  }
  
  bool
  attemptMove(move attemptThisMove){

    int fromPos = attemptThisMove.from;
    int toPos = attemptThisMove.to;
    
    int fromX = fromPos%8;
    int fromY = fromPos/8;
    int toX = toPos%8;
    int toY = toPos/8;
    
    
    if(!((fromX >= 0)&&(fromX < 8)&&(fromY >= 0)&&(fromY < 8))){
      return false;
    }
    if(!((toX >= 0)&&(toX < 8)&&(toY >= 0)&&(toY < 8))){
      return false;
    }
    
    char promotion = attemptThisMove.promotion;
    if(promotion != 0){
      if(currentState.isWhitesTurn){
	assert((toY == 0)&&(currentState.board[fromPos] == WP));
      }else{
	assert((toY == 7)&&(currentState.board[fromPos] == BP));
      }
    }else{
      if(currentState.isWhitesTurn){
	assert(!((toY == 0)&&(currentState.board[fromPos] == WP)));
      }else{
	assert(!((toY == 7)&&(currentState.board[fromPos] == BP)));
      }
    }
    bool success = false;
    
    std::vector<move> legalMoves = chessGame::generateLegalMoves(&currentState);
    for(int i = 0; i < (int)legalMoves.size(); i++){
      if((fromPos == legalMoves[i].from)&&(toPos == legalMoves[i].to)&&(promotion == legalMoves[i].promotion)){
	success = true;
      }
    }
    if(success){
      forceMove(attemptThisMove, &currentState);
      backupCurrentState();
    }
    
    return success;
  }
};


int
nodeTree(int depth, boardState state){
  if(depth == 0){
    return 1;
  }

  int numStatesOnBranch = 0;
  std::vector<move> moveList = chessGame::generateLegalMoves(&state);
  for(int i = 0; i < (int)moveList.size(); i++){
    boardState tmp = state;
    chessGame::forceMove(moveList[i], &tmp);
    numStatesOnBranch += nodeTree(depth-1, tmp);
  }

  return numStatesOnBranch;
}

int
nodeTest(int depth, boardState state){
  int total = 0;
  std::vector<move> moveList = chessGame::generateLegalMoves(&state);

  
  for(int i = 0; i < (int)moveList.size(); i++){
    boardState tmp = state;
    chessGame::forceMove(moveList[i], &tmp);
    int thisBranch = nodeTree(depth-1, tmp);
    total += thisBranch;
    
    
    int fromX = moveList[i].from%8;
    int fromY = moveList[i].from/8;
    int toX = moveList[i].to%8;
    int toY = moveList[i].to/8;
    printf("%c%c%c%c = %d\n", 'a' + fromX, '8' - fromY, 'a' + toX, '8' - toY, thisBranch);
  }
  return total;
}

void
generateDistanceToEdge(void){
  for(int startPos = 0; startPos < 64; startPos++){
    for(int dirNum  = 0; dirNum < 8; dirNum++){
      for(int i = 0; i < 100; i++){
	
	
	int x = (startPos%8) + i*actualDirections[dirNum][0];
	int y = (startPos/8) + i*actualDirections[dirNum][1];
	if((x < 0)||(x >= 8)||(y < 0)||(y >= 8)){
	  numSquaresTillEdge[startPos][dirNum] = i;
	  break;
	}
      }
    }
  }
}
