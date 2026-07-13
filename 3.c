#include "raylib.h"

#define TILE_SIZE 40
#define MAP_WIDTH 20
#define MAP_HEIGHT 15
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define MAX_SCROLLS 3
#define MAX_LEVELS 2
#define MAX_GUARDS 2  

typedef enum {
    STATE_GAMEPLAY,
    STATE_RESTORATION,
    STATE_GAME_OVER,
    STATE_VICTORY,
    STATE_CAMPAIGN_COMPLETE
} GameState;

typedef struct {
    int gridX;
    int gridY;
    int scrollsCarried;
    int scrollsDelivered;
} Player;

typedef struct {
    int gridX;
    int gridY;
    bool isSpawned;
    char title[50];
    char Fact[200];
} Manuscript;

typedef struct {
    int gridX;
    int gridY;
    int startGridX;
    int endGridX;
    int direction;
    float moveTimer;
    float moveDelay;
    bool isStalker;  
} Guard;

// --- NEW ANIMATION STRUCTURE ---
typedef struct {
    int gridX;
    int gridY;
    float timer;
    float duration;
    bool isActive;
} FallingWall;

int campaignMaps[MAX_LEVELS][MAP_HEIGHT][MAP_WIDTH] = {
    { // --- LEVEL 0: Winding Alleys ---
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,0,1,0,1,1,1,1,1,1,1,1,0,1,1,1,0,1},
        {1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1},
        {1,0,1,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1},
        {1,0,0,0,0,0,0,1,0,1,3,1,0,1,0,1,0,0,0,1}, 
        {1,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1},
        {1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,1},
        {1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1},
        {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1},
        {1,1,1,1,1,1,0,1,0,1,0,1,1,0,0,1,1,1,0,1},
        {1,0,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1},
        {1,0,1,1,0,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1},
        {1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    { // --- LEVEL 1: Outstanding Courtyard Architecture ---
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,2,2,0,0,0,0,2,2,0,0,0,0,0,1},
        {1,0,4,4,4,0,2,2,0,1,3,1,0,2,2,0,4,4,0,1},
        {1,0,4,4,4,0,0,0,0,0,0,0,0,0,0,0,4,4,0,1},
        {1,0,4,4,4,0,1,1,1,2,2,1,1,1,0,0,4,4,0,1},
        {1,0,0,0,0,0,1,4,4,2,2,4,4,1,0,0,0,0,0,1},
        {1,1,1,2,2,1,1,4,4,0,0,4,4,1,1,2,2,1,1,1},
        {1,0,0,2,2,0,0,0,0,0,0,0,0,0,0,2,2,0,0,1}, 
        {1,0,4,2,2,4,4,1,1,0,0,1,1,4,4,2,2,4,0,1},
        {1,0,4,0,0,4,4,1,0,0,0,0,1,4,4,0,0,4,0,1},
        {1,0,4,0,0,4,4,1,0,1,1,0,1,4,4,0,0,4,0,1},
        {1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1}, 
        {1,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    }
};

// Global level-0 reset map to handle game-overs cleanly
int levelZeroBackup[MAP_HEIGHT][MAP_WIDTH];

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Search For Wisdoms - Dynamic Alleys");
    SetTargetFPS(60);
    
    // Backup the initial level 0 layout configuration
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            levelZeroBackup[y][x] = campaignMaps[0][y][x];
        }
    }

    GameState CurrentState = STATE_GAMEPLAY;
    int currentLevel = 0;
    
    Player player = { .gridX = 1, .gridY = 1, .scrollsCarried = 0, .scrollsDelivered = 0 };
    
    // Variables to track movement transitions
    int lastPlayerGridX = 1;
    int lastPlayerGridY = 1;
    FallingWall fallingTile = { .isActive = false };

    // --- SQUASHED BUGS: Added missing commas, brackets, and corrected structure syntax ---
    Manuscript activeScrolls[MAX_LEVELS][MAX_SCROLLS] = {
        { // --- LEVEL 1 SCROLLS (Hadiths) ---
            { .gridX = 1, .gridY = 13, .isSpawned = true, .title = "Sayings of The Prophet (pbuh)", .Fact = "Richness is not having many possessions, \nbut true richness is the richness of the soul." },
            { .gridX = 18, .gridY = 1, .isSpawned = true, .title = "Sayings of The Prophet (pbuh)", .Fact = "Part of the perfection of one's Islam is his leaving \n that which does not concern him." },
            { .gridX = 1, .gridY = 7, .isSpawned = true, .title = "Sayings of The Prophet (pbuh)", .Fact = "None of you [truly] believes until \nhe loves for his brother that which he loves for himself." }
        },
        { // --- LEVEL 2 SCROLLS (Hadiths) ---
            { .gridX = 1, .gridY = 13, .isSpawned = true, .title = "Sayings of The Prophet (pbuh)", .Fact = "Let him who believes in Allah and \nthe Last Day speak good, or keep silent." },
            { .gridX = 18, .gridY = 1, .isSpawned = true, .title = "Sayings of The Prophet (pbuh)", .Fact = "A believer is not stung twice \nout of one and the same hole." },
            { .gridX = 8, .gridY = 11, .isSpawned = true, .title = "Sayings of The Prophet (pbuh)", .Fact = "Be in this world as though \nyou were a stranger or a wayfarer." }
        }
    };
    
    int currentScrollIndex = 0;

    Guard guards[MAX_GUARDS] = {
        { .gridX = 13, .gridY = 7, .startGridX = 1, .endGridX = 18, .direction = 1, .moveTimer = 0.0f, .moveDelay = 0.20f, .isStalker = false },
        { .gridX = 10, .gridY = 11, .startGridX = 0, .endGridX = 0, .direction = 0, .moveTimer = 0.0f, .moveDelay = 0.35f, .isStalker = true } 
    };

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        
        switch (CurrentState) {
            case STATE_GAMEPLAY: {
                // Record position before evaluating input steps
                lastPlayerGridX = player.gridX;
                lastPlayerGridY = player.gridY;

                int nextX = player.gridX;
                int nextY = player.gridY;

                if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) nextX += 1;
                if (IsKeyPressed(KEY_LEFT)  || IsKeyPressed(KEY_A)) nextX -= 1;
                if (IsKeyPressed(KEY_DOWN)  || IsKeyPressed(KEY_S)) nextY += 1;
                if (IsKeyPressed(KEY_UP)    || IsKeyPressed(KEY_W)) nextY -= 1;

                if (nextX >= 0 && nextX < MAP_WIDTH && nextY >= 0 && nextY < MAP_HEIGHT) {
                    if (campaignMaps[currentLevel][nextY][nextX] != 1 && campaignMaps[currentLevel][nextY][nextX] != 4) {
                        player.gridX = nextX;
                        player.gridY = nextY;
                    }
                }

                // --- CRITICAL FALLING WALL TRIGGER LOGIC (Level 0 Only) ---
                if (currentLevel == 0 && (player.gridX != lastPlayerGridX || player.gridY != lastPlayerGridY)) {
                    // Only drop a wall if the previous tile wasn't the spawn location or a goal zone
                    if (campaignMaps[0][lastPlayerGridY][lastPlayerGridX] == 0) {
                        fallingTile.gridX = lastPlayerGridX;
                        fallingTile.gridY = lastPlayerGridY;
                        fallingTile.timer = 0.0f;
                        fallingTile.duration = 0.4f; // Wall takes 0.4 seconds to visibly land
                        fallingTile.isActive = true;
                    }
                }

                // --- ANIMATION STEP PROCESSING ---
                if (fallingTile.isActive) {
                    fallingTile.timer += dt;
                    if (fallingTile.timer >= fallingTile.duration) {
                        // Animation complete: Permanently commit a solid brick wall tile (1) to map matrix
                        campaignMaps[0][fallingTile.gridY][fallingTile.gridX] = 1;
                        fallingTile.isActive = false;
                    }
                }

                // MULTI-LEVEL SCROLL INTERSECTIONS CHECK
                for (int i = 0; i < MAX_SCROLLS; i++) {
                    if (activeScrolls[currentLevel][i].isSpawned && player.gridX == activeScrolls[currentLevel][i].gridX && player.gridY == activeScrolls[currentLevel][i].gridY) {   
                        currentScrollIndex = i;
                        CurrentState = STATE_RESTORATION;
                        break;
                    }
                }

                if (campaignMaps[currentLevel][player.gridY][player.gridX] == 3 && player.scrollsCarried > 0) {
                    player.scrollsDelivered += player.scrollsCarried;
                    player.scrollsCarried = 0;

                    if (player.scrollsDelivered >= MAX_SCROLLS) {
                        if (currentLevel + 1 < MAX_LEVELS) {
                            CurrentState = STATE_VICTORY;
                        } else {
                            CurrentState = STATE_CAMPAIGN_COMPLETE;
                        }
                    }
                }
        
                int activeGuardsThisLevel = (currentLevel == 0) ? 1 : MAX_GUARDS;
                
                for (int i = 0; i < activeGuardsThisLevel; i++) {
                    guards[i].moveTimer += dt;
                    if (guards[i].moveTimer >= guards[i].moveDelay) {
                        guards[i].moveTimer = 0.0f;
                        
                        if (!guards[i].isStalker) {
                            int guardNextX = guards[i].gridX + guards[i].direction;
                            if (guardNextX > guards[i].endGridX || guardNextX < guards[i].startGridX || campaignMaps[currentLevel][guards[i].gridY][guardNextX] == 1) {
                                guards[i].direction *= -1;
                            } else {
                                guards[i].gridX = guardNextX;
                            }
                        } else {
                            int targetStepX = guards[i].gridX;
                            int targetStepY = guards[i].gridY;

                            if (player.gridX > guards[i].gridX) targetStepX++;
                            else if (player.gridX < guards[i].gridX) targetStepX--;

                            if (player.gridY > guards[i].gridY) targetStepY++;
                            else if (player.gridY < guards[i].gridY) targetStepY--;

                            // FIXED SQUASHED BUG: Added missing closing parenthesis 
                            if (campaignMaps[currentLevel][targetStepY][targetStepX] != 1) {
                                guards[i].gridX = targetStepX;
                                guards[i].gridY = targetStepY;
                            }
                        }
                    }
                    
                    if (player.gridX == guards[i].gridX && player.gridY == guards[i].gridY) {
                        CurrentState = STATE_GAME_OVER;
                    }
                }
            } break;
        
            case STATE_RESTORATION: {
                if (IsKeyPressed(KEY_SPACE)) {
                    player.scrollsCarried++;
                    activeScrolls[currentLevel][currentScrollIndex].isSpawned = false; 
                    CurrentState = STATE_GAMEPLAY;  
                }
            } break;

            case STATE_GAME_OVER: {
                if (IsKeyPressed(KEY_ENTER)) {
                    player.gridX = 1; player.gridY = 1;
                    player.scrollsCarried = 0; player.scrollsDelivered = 0;
                    fallingTile.isActive = false;

                    // Revert Level 0 structural map configurations back to original empty pathways
                    for (int y = 0; y < MAP_HEIGHT; y++) {
                        for (int x = 0; x < MAP_WIDTH; x++) {
                            campaignMaps[0][y][x] = levelZeroBackup[y][x];
                        }
                    }

                    for (int i = 0; i < MAX_SCROLLS; i++) activeScrolls[currentLevel][i].isSpawned = true;
                    
                    guards[0].gridX = 13; guards[0].direction = 1;
                    guards[1].gridX = 10; guards[1].gridY = 11; 
                    CurrentState = STATE_GAMEPLAY;
                }
            } break;

            case STATE_VICTORY: {
                if (IsKeyPressed(KEY_ENTER)) {
                    currentLevel++;
                    player.gridX = 1; player.gridY = 1;
                    player.scrollsCarried = 0; player.scrollsDelivered = 0;
                    guards[0].gridX = 13; guards[0].direction = 1;
                    guards[1].gridX = 10; guards[1].gridY = 11;
                    CurrentState = STATE_GAMEPLAY;
                }
            } break;

            case STATE_CAMPAIGN_COMPLETE: {
                if (IsKeyPressed(KEY_ENTER)) {
                    currentLevel = 0;
                    player.gridX = 1; player.gridY = 1;
                    player.scrollsCarried = 0; player.scrollsDelivered = 0;
                    fallingTile.isActive = false;

                    for (int y = 0; y < MAP_HEIGHT; y++) {
                        for (int x = 0; x < MAP_WIDTH; x++) {
                            campaignMaps[0][y][x] = levelZeroBackup[y][x];
                        }
                    }

                    for (int lvl = 0; lvl < MAX_LEVELS; lvl++) {
                        for (int i = 0; i < MAX_SCROLLS; i++) activeScrolls[lvl][i].isSpawned = true;
                    }
                    CurrentState = STATE_GAMEPLAY;
                }
            } break;
        }

        // =================================================
        // 2. RENDER GRAPHICS SYSTEM
        // =================================================
        BeginDrawing();
            ClearBackground(RAYWHITE);

            switch (CurrentState) {
                case STATE_GAMEPLAY: {
                    for (int y = 0; y < MAP_HEIGHT; y++) {
                        for (int x = 0; x < MAP_WIDTH; x++) {
                            Rectangle tileRect = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                            
                            int tileType = campaignMaps[currentLevel][y][x];
                            if (tileType == 1) {
                                DrawRectangleRec(tileRect, DARKBROWN); 
                            } else if (tileType == 2) {
                                DrawRectangleRec(tileRect, SKYBLUE);   
                            } else if (tileType == 4) {
                                DrawRectangleRec(tileRect, (Color){144, 238, 144, 255}); 
                            } else if (tileType == 3) {
                                DrawRectangleRec(tileRect, (Color){80,200,120,255}); 
                            }
                            DrawRectangleLines(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, LIGHTGRAY);
                        }
                    }
                    
                    // --- RENDERING ACTIVE FALLING WALL TRANSITION ---
                    if (currentLevel == 0 && fallingTile.isActive) {
                        float progress = fallingTile.timer / fallingTile.duration;
                        // Linear size expansion simulation over running frames
                        int currentSize = (int)(TILE_SIZE * progress);
                        int offset = (TILE_SIZE - currentSize) / 2;
                        
                        DrawRectangle(
                            fallingTile.gridX * TILE_SIZE + offset, 
                            fallingTile.gridY * TILE_SIZE + offset, 
                            currentSize, currentSize, ORANGE
                        );
                    }

                    for (int i = 0; i < MAX_SCROLLS; i++) {
                        if (activeScrolls[currentLevel][i].isSpawned) {
                            DrawRectangle(activeScrolls[currentLevel][i].gridX * TILE_SIZE + 12, activeScrolls[currentLevel][i].gridY * TILE_SIZE + 12, 16, 16, GOLD);
                        }
                    }

                    int activeGuardsThisLevel = (currentLevel == 0) ? 1 : MAX_GUARDS;
                    for (int i = 0; i < activeGuardsThisLevel; i++) {
                        int gPixelX = guards[i].gridX * TILE_SIZE + TILE_SIZE / 2;
                        int gPixelY = guards[i].gridY * TILE_SIZE + TILE_SIZE / 2;
                        DrawCircle(gPixelX, gPixelY, 14, guards[i].isStalker ? PURPLE : RED);
                    }

                    int pixelX = player.gridX * TILE_SIZE + TILE_SIZE / 2;
                    int pixelY = player.gridY * TILE_SIZE + TILE_SIZE / 2;
                    DrawCircle(pixelX, pixelY, 14, BLUE);

                    DrawText(TextFormat("Level: %d/2 | Carrying: %d", currentLevel + 1, player.scrollsCarried), 15, 15, 18, BLACK);
                    DrawText(TextFormat("Secured: %d/%d", player.scrollsDelivered, MAX_SCROLLS), 550, 15, 18, DARKGREEN);
                } break;

                case STATE_RESTORATION: {
                    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){30,30,30,255});
                    DrawText("WISDOM UNCOVERED", 240, 100, 32, GOLD);
                    DrawText(activeScrolls[currentLevel][currentScrollIndex].title, 80, 200, 24, RAYWHITE);
                    DrawText(activeScrolls[currentLevel][currentScrollIndex].Fact, 80, 260, 18, LIGHTGRAY);
                    DrawText("Press [SPACEBAR] to index this scroll into inventory", 160, 480, 16, GREEN);
                } break;

                case STATE_GAME_OVER: {
                    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
                    DrawText("CAUGHT BY THE GUARD PATROL", 160, 220, 30, RED);
                    DrawText("Patience is the foundation of true learning.", 210, 290, 18, LIGHTGRAY);
                    DrawText("Press [ENTER] to return to the streets", 240, 440, 16, GOLD);
                } break;

                case STATE_VICTORY: {
                    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){80,200,120,255});
                    DrawText("LEVEL ACCOMPLISHED!", 230, 200, 34, GOLD);
                    DrawText("The scrolls are safe inside the Bayt al-Hikma archives.", 100, 270, 18, RAYWHITE);
                    DrawText("Press [ENTER] to advance to the next level quadrant", 210, 450, 16, DARKGRAY);
                } break;
                
                case STATE_CAMPAIGN_COMPLETE: {
                    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GOLD);
                    DrawText("CAMPAIGN COMPLETE!", 230, 200, 34, PURPLE);
                    DrawText("All Wisdoms has been fully preserved.", 150, 270, 18, BLACK);
                    DrawText("Press [ENTER] to loop simulation", 260, 450, 16, DARKGRAY);
                } break;
            }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}