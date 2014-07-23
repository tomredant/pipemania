#include "Board.h"
#include "Pipe.h"

const int Board::x_offset = 227;
const int Board::y_offset = 35;
const int Board::slotSize = 48;
const int Board::lines = BOARD_LINES;
const int Board::columns = BOARD_COLUMNS;

Board::Board (SDL_Surface* s, SDL_Rect* c, SDL_Surface* back, SDL_Surface* pipe1, SDL_Surface* pipe2)
{
    screen = s;
    coordinates = c;
    background = back;
    pipes_sprite1 = pipe1;
    pipes_sprite2 = pipe2;
    starting_time = SDL_GetTicks();
    flow_started = false;
    game_in_progress = true;

    for (int line = 0; line < lines; line++) {
        for (int column = 0; column < columns; column++) {
            slots[line][column] = NULL;
        }
    }

    for (int p = 0; p < POOL_SIZE; p++) {
        pool[p] = new Pipe(pipes_sprite1, pipes_sprite2);
    }
}

void Board::mouseClick (int x, int y)
{
    int x_min = x_offset, x_max = x_min + (lines * slotSize);
    int y_min = y_offset, y_max = y_min + (columns * slotSize);

    // Check limits
    if (x >= x_min && x <= x_max && y >= y_min && y <= y_max) {
        int line = (x - x_min) / slotSize;
        int column = (y - y_min) / slotSize;
        Pipe **pipe = &slots[line][column];

        if (*pipe) {
            if ((*pipe)->isBlocked())
                return;

            delete *pipe;
        }

        // Get top of the pool
        *pipe = pool[0];
        rotatePool();
    }
}

void Board::rotatePool (void)
{
    for (int p = 0; p < POOL_SIZE - 1; p++) {
        pool[p] = pool[p + 1];
    }

    pool[POOL_SIZE - 1] = new Pipe(pipes_sprite1, pipes_sprite2);
}

SDL_Rect Board::getSlotScreenPosition (int line, int column)
{
    SDL_Rect pos;

    pos.x = (line * slotSize) + x_offset;
    pos.y = (column * slotSize) + y_offset;

    return pos;
}

Pipe* Board::getCurrentPipe() {
    if(current_pipe_column >= BOARD_COLUMNS || current_pipe_column < 0 ||
       current_pipe_line >= BOARD_LINES || current_pipe_line < 0) {
       return NULL;
    } else {
        return slots[current_pipe_column][current_pipe_line];
    }
}

void Board::Update() {
    if(game_in_progress) {
        updatePipes();
        updateStartingFlow();
        updateNextPipe();
    }
}

void Board::updatePipes() {
    for (int l = 0; l < lines; l++) {
        for (int c = 0; c < columns; c++) {
            if (slots[l][c] != NULL) {
                slots[l][c]->Update();
            }
        }
    }
}

void Board::updateStartingFlow() {
    if (flow_started == false && SDL_GetTicks() - starting_time > INITIAL_DELAY) {
        if (slots[INITIAL_COLUMN][INITIAL_LINE] != NULL) {
            current_pipe_column = INITIAL_COLUMN;
            current_pipe_line = INITIAL_LINE;
            getCurrentPipe()->StartFlow(FLOW_LEFT);
            flow_started = true;
        } else {
            gameOver();
        }
    }
}

void Board::updateNextPipe() {
    if (flow_started == true && getCurrentPipe()->isFlowFinished()) {
        int flow_direction = getCurrentPipe()->getFlowTurnPosition();
        int next_flow;

        switch(flow_direction) {
            case FLOW_TOP:
                current_pipe_line -= 1;
                next_flow = FLOW_DOWN;
                break;
            case FLOW_RIGHT:
                current_pipe_column += 1;
                next_flow = FLOW_LEFT;
                break;
            case FLOW_DOWN:
                current_pipe_line += 1;
                next_flow = FLOW_TOP;
                break;
            case FLOW_LEFT:
                current_pipe_column -= 1;
                next_flow = FLOW_RIGHT;
                break;
        }

        // game over if has no next pipe or the next pipe does not have the next_flow entry
        if (getCurrentPipe() == NULL || !getCurrentPipe()->hasFlowEntry(next_flow)) {
            gameOver();
        } else {
            getCurrentPipe()->StartFlow(next_flow);
        }
    } else if(flow_started == true) {
        getCurrentPipe()->setFlowTurnPosition(calculateNextFlowDirection());
    }
}

int Board::calculateNextFlowDirection() {
    return FLOW_RIGHT;
}

void Board::Draw ()
{
    // Draw background
    SDL_BlitSurface(background, 0, screen, coordinates);

    // Draw all board pipes
    for (int l = 0; l < lines; l++) {
        for (int c = 0; c < columns; c++) {
            // if != NULL we have a pipe to draw
            if (slots[l][c] != NULL) {
                SDL_Rect pos = getSlotScreenPosition(l, c);
                slots[l][c]->Draw(screen, &pos);
            }
        }
    }

    // Draw pool pipes
    SDL_Rect pos = {
        .x = POOL_OFFSET_X,
        .y = POOL_OFFSET_Y,
    };

    for (int p = POOL_SIZE - 1; p > 0; p--, pos.y += POOL_SPACING) {
        pool[p]->Draw(screen, &pos);
    }

    pos.y = POOL_TOP_Y;
    pool[0]->Draw(screen, &pos);
}

void Board::gameOver() {
    game_in_progress = false;
}
