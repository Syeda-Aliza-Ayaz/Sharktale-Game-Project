#include "raylib.h"

#define MAX_OBSTACLES 10

typedef struct {
    Texture2D texture;
    Vector2 position;
    float speed;
    bool isActive;
    Rectangle hitbox;
} Obstacle;

// Global audio state
bool soundEnabled = true;

// Function to toggle sound state and control audio playback
void ToggleSound(bool enableSound, Music *mainScreenMusic, Music *levelsScreenMusic) {
    soundEnabled = enableSound; // Update global state

    if (soundEnabled) {
        ResumeMusicStream(*mainScreenMusic);
        ResumeMusicStream(*levelsScreenMusic);
    } else {
        PauseMusicStream(*mainScreenMusic);
        PauseMusicStream(*levelsScreenMusic);
    }
}

void LoadGameTextures(Texture2D* sharkFrames, int frameCount, Texture2D* orig_big, 
                      Texture2D* sharkNet, Texture2D* stingray, Texture2D* smallFish, 
                      Texture2D* bigFish, Texture2D* seahorse, Texture2D* Won, Texture2D* Lose, Texture2D*Instructions) {
    // Load the shark frames for animation
    sharkFrames[0] = LoadTexture("Shark1.png");
    sharkFrames[1] = LoadTexture("Shark2.png");
    sharkFrames[2] = LoadTexture("Shark3.png");
    sharkFrames[3] = LoadTexture("Shark4.png");

    // Load background texture
    *orig_big = LoadTexture("bg.png");

    // Load obstacle textures
    *sharkNet = LoadTexture("bomb.png");
    *stingray = LoadTexture("anchor.png");
    *smallFish = LoadTexture("sfish.png");
    *bigFish = LoadTexture("bfish2.png");
    *seahorse = LoadTexture("starfish.png");

    // Load Won and Lose Screens
    *Instructions = LoadTexture("Artboard 2.png");
    *Lose = LoadTexture("Artboard 3.png");
    *Won = LoadTexture("Artboard 4.png");
}

void LoadGameAudio(Music *mainScreenMusic,Music *levelsScreenMusic,Sound *winningmusic, Sound *loseSound1, Sound *loseSound2 ) {
    // Load music and sound effects
    *mainScreenMusic = LoadMusicStream("mainscreen.mp3"); // Looping music
    *loseSound1 = LoadSound("game-over-music.mp3");       // Sound effect when losing
    *loseSound2 = LoadSound("game-over-male.mp3");        // Sound of male when losing
    
    *levelsScreenMusic = LoadMusicStream("levels-screen-music.mp3"); 
    *winningmusic = LoadSound("winning music.mp3");

    SetSoundVolume(*loseSound1, 1.0f); // Set loseSound1 to half volume
    SetSoundVolume(*loseSound2, 2.0f); // Set loseSound2 to full volume (louder)

    // Start playing main screen music
    PlayMusicStream(*mainScreenMusic);
    (*mainScreenMusic).looping = true; // Ensure it loops
    
     PlayMusicStream(*levelsScreenMusic);
    (*levelsScreenMusic).looping = true;
}

int HandleCollision(Texture2D sharkNet, Music mainScreenMusic, Sound loseSound1, Sound loseSound2, Texture2D Lose, Texture2D obstacleTexture, int *score, int *level, float *sharkSpeed, float *sharkX, float *sharkY, float screenHeight, Texture2D *sharkFrames,Obstacle obstacles[], int screenWidth, Texture2D stingray, Texture2D smallFish, Texture2D bigFish, Texture2D seahorse) {
    
    if (obstacleTexture.id == sharkNet.id) {
        
        Rectangle RestartButton = { 1010, 525, 94, 95};
        *score = 0;
        *level = 1;
        *sharkSpeed = 5.0f;
        
        *sharkX = 100.0f;
        *sharkY = screenHeight / 2.0f - sharkFrames[0].height / 2.0f;
        
        InitializeObstacles(obstacles, screenWidth, screenHeight, sharkNet, stingray, smallFish, bigFish, seahorse, 7, 11);
        
        PauseMusicStream(mainScreenMusic);
        PlaySound(loseSound1); // Play losing sound
        PlaySound(loseSound2); // Play losing sound
        
        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawTexture(Lose, 0, 0, WHITE);
            
            Vector2 mousePoint = GetMousePosition();
        
                if (CheckCollisionPointRec(mousePoint, RestartButton)) {

                    // If the play button is clicked
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                      
                        break;
                    }
                }                
   
            EndDrawing();
        }
        ResumeMusicStream(mainScreenMusic);
    }
    return 1; // Indicate no collision
}

void UpdateSharkPosition(float *sharkX, float *sharkY, float sharkSpeed) {
    if (IsKeyDown(KEY_UP)) *sharkY -= sharkSpeed;
    if (IsKeyDown(KEY_DOWN)) *sharkY += sharkSpeed;
    if (IsKeyDown(KEY_LEFT)) *sharkX -= sharkSpeed;
    if (IsKeyDown(KEY_RIGHT)) *sharkX += sharkSpeed;
}

void ConstrainSharkWithinBounds(float *sharkX, float *sharkY, float screenWidth, float screenHeight, Texture2D *sharkFrames, float scaleFactor) {
    if (*sharkY < 0) *sharkY = 0;
    if (*sharkY + sharkFrames[0].height * scaleFactor > screenHeight) 
        *sharkY = screenHeight - sharkFrames[0].height * scaleFactor;

    if (*sharkX < 0) *sharkX = 0;
    if (*sharkX + sharkFrames[0].width * scaleFactor > screenWidth) 
        *sharkX = screenWidth - sharkFrames[0].width * scaleFactor;
}

void UpdateBackground(float *backgroundX, float backgroundSpeed, float screenWidth) {
    *backgroundX -= backgroundSpeed;
    if (*backgroundX <= -screenWidth) *backgroundX = 0;
}

void UpdateAnimationFrame(int *frameCounter, int *currentFrame, int frameCount, int frameSpeed) {
    (*frameCounter)++;
    if (*frameCounter >= (60 / frameSpeed)) {
        *frameCounter = 0;
        (*currentFrame)++;
        if (*currentFrame >= frameCount) *currentFrame = 0;
    }
}

void ResetObstaclePosition(Obstacle obstacles[], Rectangle sharkHitbox, bool *isImmune, float *immunityTimer, int *score, Texture2D sharkNet, Texture2D stingray, Texture2D smallFish, Texture2D bigFish, Texture2D seahorse, Music mainScreenMusic, Sound loseSound1, Sound loseSound2, Texture2D Lose, int screenWidth, int screenHeight, int x, int y, int *level, float *sharkSpeed,float *sharkX, float *sharkY, Texture2D *sharkFrames) {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (obstacles[i].isActive) {
            // Move obstacle to the left
            obstacles[i].position.x -= obstacles[i].speed;

            // Reset obstacle when it moves off-screen
            if (obstacles[i].position.x + obstacles[i].texture.width < 0) {
                obstacles[i].position.x = screenWidth + GetRandomValue(0, 200);
                obstacles[i].position.y = GetRandomValue(0, screenHeight - obstacles[i].texture.height);
                obstacles[i].speed = GetRandomValue(x, y);

                // Randomize obstacle type
                int obstacleType = GetRandomValue(0, 9);
                switch (obstacleType) {
                    case 0:
                    case 1: 
                        obstacles[i].texture = sharkNet; 
                        break;
                    case 2:
                    case 3: 
                        obstacles[i].texture = stingray; 
                        break;
                    case 4:
                    case 5: 
                        obstacles[i].texture = smallFish; 
                        break;
                    case 6:
                    case 7: 
                        obstacles[i].texture = bigFish; 
                        break;
                    case 8: 
                        obstacles[i].texture = seahorse; 
                        break;
                }
            }

            // Update hitbox for collision detection
            obstacles[i].hitbox.x = obstacles[i].position.x + 7;
            obstacles[i].hitbox.y = obstacles[i].position.y + 7;

            // Check for collision with shark
            if (CheckCollisionRecs(sharkHitbox, obstacles[i].hitbox)) {
                if (!(*isImmune)) {
                    // Handle obstacle collision
                    if (obstacles[i].texture.id == bigFish.id) {
                        *score += 10; // Score for big fish
                    } else if (obstacles[i].texture.id == smallFish.id) {
                        *score += 5; // Score for small fish
                    } else if (!HandleCollision(sharkNet, mainScreenMusic, loseSound1, loseSound2, Lose, obstacles[i].texture, score, level, sharkSpeed,sharkX,sharkY, screenHeight, sharkFrames, obstacles, screenWidth, stingray, smallFish, bigFish, seahorse)) {
                        return; // Exit if collision handling ends the game
                    } else if (obstacles[i].texture.id == seahorse.id) {
                        *isImmune = true; // Gain immunity
                        *immunityTimer = 3.0f; // Set immunity timer
                    } else if (obstacles[i].texture.id == stingray.id) {
                        *score -= 5; // Lose points for stingray
                        if (*score < 0) *score = 0; // Prevent negative score
                    }
                } else {
                    // If immune, continue scoring without penalties
                    if (obstacles[i].texture.id == bigFish.id) {
                        *score += 10;
                    } else if (obstacles[i].texture.id == smallFish.id) {
                        *score += 5;
                    }
                }

                // Reset obstacle after collision
                obstacles[i].position.x = screenWidth + GetRandomValue(0, 200);
                obstacles[i].position.y = GetRandomValue(0, screenHeight - obstacles[i].texture.height);
                obstacles[i].speed = GetRandomValue(x, y);
            }
        }
    }
}


void UpdateImmunity(bool *isImmune, float *immunityTimer) {
    if (*isImmune) {
        *immunityTimer -= GetFrameTime(); // Decrease immunity timer based on frame time
        if (*immunityTimer <= 0.0f) {
            *isImmune = false; // Disable immunity when the timer expires
            *immunityTimer = 0.0f; // Ensure timer does not go below zero
        }
    }
}

// Helper function to initialize obstacles for a level
void InitializeObstacles(Obstacle obstacles[], int screenWidth, int screenHeight, Texture2D sharkNet, Texture2D stingray, 
                         Texture2D smallFish, Texture2D bigFish, Texture2D seahorse, int minSpeed, int maxSpeed) {
    
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        int obstacleType = GetRandomValue(0, 9);
        switch (obstacleType) {
            case 0:
            case 1:
                obstacles[i].texture = sharkNet;
                break;
            case 2:
            case 3:
                obstacles[i].texture = stingray;
                break;
            case 4:
            case 5:
                obstacles[i].texture = smallFish;
                break;
            case 6:
            case 7:
                obstacles[i].texture = bigFish;
                break;
            case 8:
                obstacles[i].texture = seahorse;
                break;
        }

        obstacles[i].position = (Vector2){ screenWidth + i * 200, GetRandomValue(0, screenHeight - obstacles[i].texture.height) };
        obstacles[i].speed = GetRandomValue(minSpeed, maxSpeed);
        obstacles[i].isActive = true;
        obstacles[i].hitbox = (Rectangle){ obstacles[i].position.x + 7, obstacles[i].position.y + 7, obstacles[i].texture.width + 5, obstacles[i].texture.height + 10 };
    }
}

void UpdateLevelAndDraw(int *level, int *score, float *sharkSpeed, Obstacle obstacles[], int screenWidth, int screenHeight, 
                        Texture2D sharkNet, Texture2D stingray, Texture2D smallFish, Texture2D bigFish, Texture2D seahorse,
                        Texture2D orig_big, Texture2D Won, Texture2D sharkFrames[], int currentFrame, float scaleFactor, 
                        float sharkX, float sharkY, float backgroundX , bool *isDisplayingText, float *displayTime, 
                        float textDisplayDuration, bool isImmune,Sound winningmusic) {

    // Check level progression
    if (*level == 1 && *score >= 300) {
        *level = 2;
        *score = 0;
        *sharkSpeed = 6.5f;
        InitializeObstacles(obstacles, screenWidth, screenHeight, sharkNet, stingray, smallFish, bigFish, seahorse, 7, 11);
        *isDisplayingText = true;
        *displayTime = 0.0f;

    } else if (*level == 2 && *score >= 150) {
        *level = 3;
        *score = 0;
        *sharkSpeed = 8.0f;
        InitializeObstacles(obstacles, screenWidth, screenHeight, sharkNet, stingray, smallFish, bigFish, seahorse, 10, 15);
        *isDisplayingText = true;
        *displayTime = 0.0f;

    } else if (*level == 3 && *score >= 100) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTexture(Won, 0, 0, WHITE);
        PlaySound(winningmusic); 
        EndDrawing();
        sleep(2);
        return; // Exit function after displaying win screen
    }

    // Update level display text timer
    if (*isDisplayingText) {
        *displayTime += GetFrameTime();
        if (*displayTime >= textDisplayDuration) {
            *isDisplayingText = false;
        }
    }

    // Begin drawing
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw background
    DrawTexture(orig_big, backgroundX, 0, WHITE);
    DrawTexture(orig_big, backgroundX + screenWidth, 0, WHITE);

    // Draw shark
    DrawTextureEx(sharkFrames[currentFrame], (Vector2){sharkX, sharkY}, 0.0f, scaleFactor, WHITE);

    // Draw obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (obstacles[i].isActive) {
            DrawTexture(obstacles[i].texture, (int)obstacles[i].position.x, (int)obstacles[i].position.y, WHITE);
        }
    }

    // Draw score and level
    DrawText(TextFormat("Score: %d", *score), 10, 10, 20, BLACK);
    DrawText(TextFormat("Level: %d", *level), screenWidth - 100, 10, 20, BLACK);

    // Draw level text if displaying
    if (*level == 2 && *isDisplayingText) {
        DrawText("Level 2", screenWidth / 2 - 50, screenHeight / 2 - 10, 40, ORANGE);
    } else if (*level == 3 && *isDisplayingText) {
        DrawText("Level 3", screenWidth / 2 - 50, screenHeight / 2 - 10, 40, ORANGE);
    }

    // Draw immunity status
    if (isImmune) {
        DrawText("Immunity Active!", screenWidth / 2 - 70, screenHeight / 2 - 50, 20, GREEN);
    }

    EndDrawing();
}


int main(void) {
    // Initialization
    const int screenWidth = 1200;
    const int screenHeight = 675;

    InitWindow(screenWidth, screenHeight, "Shark Game");
    // Initialize audio files
    InitAudioDevice(); 
    
    // Load the shark frames for animation
    const int frameCount = 4;  
    Texture2D sharkFrames[frameCount];
    // Declare Textures
    Texture2D orig_big, sharkNet, stingray, smallFish, bigFish, seahorse, Won, Lose, Instructions;    
    // Call the function to load textures
    LoadGameTextures(sharkFrames, frameCount, &orig_big, &sharkNet, &stingray, &smallFish, &bigFish, &seahorse, &Won, &Lose, &Instructions);
    
    // Music and sound effects
    Music mainScreenMusic,levelsScreenMusic;
    Sound loseSound1, loseSound2,winningmusic;

    // Load all game audio
    LoadGameAudio(&mainScreenMusic,&levelsScreenMusic,&winningmusic, &loseSound1, &loseSound2);

    
    float sharkX = 100.0f;
    float sharkY = screenHeight / 2.0f - sharkFrames[0].height / 2.0f;
    float sharkSpeed = 5.0f;

    float scaleFactor = 4.0f;

    int frameCounter = 0;
    int currentFrame = 0;
    int frameSpeed = 8;

    Obstacle obstacles[MAX_OBSTACLES];

    InitializeObstacles(obstacles, screenWidth, screenHeight, sharkNet, stingray, smallFish, bigFish, seahorse, 4, 8);

    Rectangle sharkHitbox = { sharkX + 20, sharkY + 20, sharkFrames[0].width * scaleFactor - 40, sharkFrames[0].height * scaleFactor - 40 };

    int score = 0;
    int level = 1;
    bool isImmune = false;
    float immunityTimer = 0.0f;

    float backgroundX = 0.0f;
    float backgroundSpeed = 2.0f;
    
    float displayTime = 0.0f; // Timer for displaying Level 2 text
    float textDisplayDuration = 2.0f; // Duration to display Level 2 text
    bool isDisplayingText = false; // Flag to check if we should display the text

    // Load the background image
    Texture2D background = LoadTexture("Artboard 1.png");

    // Define rectangles for buttons based on the positions of icons on the image
    // Adjust the x, y, width, and height values based on the image layout at 1200x675 resolution
    //Rectangle soundButton = { 350, 525, 85, 85 };   // Sound icon position
    Rectangle playButton = { 590, 525, 90, 80 };    // Play icon position
    Rectangle instructionsButton = { 830, 525, 85, 85 };    // Menu icon position
    Rectangle SoundButton = { 340, 520, 90, 90};
    Rectangle instplayButton = { 950, 500, 92, 90};
    
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Update the main screen music
        UpdateMusicStream(mainScreenMusic);
        
        Vector2 mousePoint = GetMousePosition();
        
        if (CheckCollisionPointRec(mousePoint, playButton)) {

            // If the play button is clicked
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                
                    level = 1;
                    score = 0;
                
                    while (!WindowShouldClose()) {
                    // Update the main screen music
                     UpdateMusicStream(levelsScreenMusic);
                    // Update shark position based on key inputs
                    UpdateSharkPosition(&sharkX, &sharkY, sharkSpeed);
                    ConstrainSharkWithinBounds(&sharkX, &sharkY, screenWidth, screenHeight, sharkFrames, scaleFactor);
                    UpdateBackground(&backgroundX, backgroundSpeed, screenWidth);
                    UpdateAnimationFrame(&frameCounter, &currentFrame, frameCount, frameSpeed);

                    sharkHitbox.x = sharkX + 20;
                    sharkHitbox.y = sharkY + 20;

                    if(level == 1){
                    
                    ResetObstaclePosition(obstacles, sharkHitbox, &isImmune, &immunityTimer, &score, sharkNet, stingray, smallFish, bigFish, seahorse, mainScreenMusic, loseSound1, loseSound2, Lose, screenWidth, screenHeight, 4, 8, &level, &sharkSpeed, &sharkX, &sharkY, sharkFrames);
                    
                    }
                    
                    if(level == 2){
                    
                    ResetObstaclePosition(obstacles, sharkHitbox, &isImmune, &immunityTimer, &score, sharkNet, stingray, smallFish, bigFish, seahorse, mainScreenMusic, loseSound1, loseSound2, Lose, screenWidth, screenHeight, 7, 11, &level, &sharkSpeed, &sharkX, &sharkY, sharkFrames);
                    
                    }
                    
                    
                    if(level == 3){
                        
                    ResetObstaclePosition(obstacles, sharkHitbox, &isImmune, &immunityTimer, &score, sharkNet, stingray, smallFish, bigFish, seahorse, mainScreenMusic, loseSound1, loseSound2, Lose, screenWidth, screenHeight, 10, 15, &level, &sharkSpeed, &sharkX, &sharkY, sharkFrames);    
                        
                    }
                    
                    UpdateImmunity(&isImmune, &immunityTimer);
                    
                    UpdateLevelAndDraw(&level, &score, &sharkSpeed, obstacles, screenWidth, screenHeight, sharkNet, stingray, smallFish, bigFish, seahorse,
                    orig_big, Won, sharkFrames, currentFrame, scaleFactor, sharkX, sharkY, backgroundX, &isDisplayingText, &displayTime,
                    textDisplayDuration, isImmune ,winningmusic);

                    
                    }
            }
        }
        
        if (CheckCollisionPointRec(mousePoint, instructionsButton)) {

            
            // If the play button is clicked
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {       
            
                while (!WindowShouldClose()) {
                    
                    UpdateMusicStream(mainScreenMusic);
                    BeginDrawing();
                    ClearBackground(RAYWHITE);
                    DrawTexture(Instructions, 0, 0, WHITE);
                    Vector2 mousePoint = GetMousePosition();
                    
                    if (CheckCollisionPointRec(mousePoint, instplayButton)) {

                        // If the play button is clicked
                        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                            
                            level = 1;
                            score = 0;
                        
                            while (!WindowShouldClose()) {
                            // Update the main screen music
                             UpdateMusicStream(levelsScreenMusic);
                            // Update shark position based on key inputs
                            UpdateSharkPosition(&sharkX, &sharkY, sharkSpeed);
                            ConstrainSharkWithinBounds(&sharkX, &sharkY, screenWidth, screenHeight, sharkFrames, scaleFactor);
                            UpdateBackground(&backgroundX, backgroundSpeed, screenWidth);
                            UpdateAnimationFrame(&frameCounter, &currentFrame, frameCount, frameSpeed);

                            sharkHitbox.x = sharkX + 20;
                            sharkHitbox.y = sharkY + 20;

                            if(level == 1){
                            
                            ResetObstaclePosition(obstacles, sharkHitbox, &isImmune, &immunityTimer, &score, sharkNet, stingray, smallFish, bigFish, seahorse, mainScreenMusic, loseSound1, loseSound2, Lose, screenWidth, screenHeight, 4, 8, &level, &sharkSpeed, &sharkX, &sharkY, sharkFrames);
                            
                            }
                            
                            if(level == 2){
                            
                            ResetObstaclePosition(obstacles, sharkHitbox, &isImmune, &immunityTimer, &score, sharkNet, stingray, smallFish, bigFish, seahorse, mainScreenMusic, loseSound1, loseSound2, Lose, screenWidth, screenHeight, 7, 11, &level, &sharkSpeed, &sharkX, &sharkY, sharkFrames);
                            
                            }
                            
                            
                            if(level == 3){
                                
                            ResetObstaclePosition(obstacles, sharkHitbox, &isImmune, &immunityTimer, &score, sharkNet, stingray, smallFish, bigFish, seahorse, mainScreenMusic, loseSound1, loseSound2, Lose, screenWidth, screenHeight, 10, 15, &level, &sharkSpeed, &sharkX, &sharkY, sharkFrames);    
                                
                            }
                            
                            UpdateImmunity(&isImmune, &immunityTimer);
                            
                            UpdateLevelAndDraw(&level, &score, &sharkSpeed, obstacles, screenWidth, screenHeight, sharkNet, stingray, smallFish, bigFish, seahorse,
                            orig_big, Won, sharkFrames, currentFrame, scaleFactor, sharkX, sharkY, backgroundX, &isDisplayingText, &displayTime,
                            textDisplayDuration, isImmune ,winningmusic);

                            
                            }
                        }
                    }
                    
                    EndDrawing();

                }
            }       
        }
        
        if (CheckCollisionPointRec(mousePoint, SoundButton)) {

            // If the play button is clicked
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                ToggleSound(!soundEnabled, &mainScreenMusic, &levelsScreenMusic);   
            }

        }        
        
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw the background image to fit the screen
        DrawTexture(background, 0, 0, WHITE);
        UpdateMusicStream(mainScreenMusic);
        
        EndDrawing();
       
    }

    // Unloading shark textures
    for (int i = 0; i < frameCount; i++) {
        UnloadTexture(sharkFrames[i]);
    }
    
    // Unload all the textures
    UnloadTexture(orig_big);
    UnloadTexture(sharkNet);
    UnloadTexture(stingray);
    UnloadTexture(smallFish);
    UnloadTexture(bigFish);
    UnloadTexture(seahorse);
    UnloadTexture(Instructions);
    UnloadTexture(Won);
    UnloadTexture(Lose);
    UnloadTexture(background);
    
    // Unload and Close Audio Files
    UnloadMusicStream(mainScreenMusic);
    UnloadMusicStream(levelsScreenMusic);
    UnloadSound(winningmusic);
    UnloadSound(loseSound1);
    UnloadSound(loseSound2);
    CloseAudioDevice();

    CloseWindow();

    return 0;
}