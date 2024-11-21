#include "raylib.h"

#define MAX_OBSTACLES 10

typedef struct {
    Texture2D texture;
    Vector2 position;
    float speed;
    bool isActive;
    Rectangle hitbox;
} Obstacle;

int main(void) {
    // Initialization
    const int screenWidth = 1200;
    const int screenHeight = 675;

    InitWindow(screenWidth, screenHeight, "Shark Game");

    // Load the shark frames for animation
    const int frameCount = 4;  // Number of frames for the shark
    Texture2D sharkFrames[frameCount];  // Array to store each shark frame
    sharkFrames[0] = LoadTexture("Shark1.png");
    sharkFrames[1] = LoadTexture("Shark2.png");
    sharkFrames[2] = LoadTexture("Shark3.png");
    sharkFrames[3] = LoadTexture("Shark4.png");

    // Load background texture
    Texture2D orig_big = LoadTexture("bg.png");

    // Load obstacle textures
    Texture2D sharkNet = LoadTexture("bomb.png");
    Texture2D stingray = LoadTexture("anchor.png"); // Replacing eel with stingray
    Texture2D smallFish = LoadTexture("sfish.png");
    Texture2D bigFish = LoadTexture("bfish2.png");
    Texture2D seahorse = LoadTexture("starfish.png");

    // Shark initial position
    float sharkX = 100.0f;
    float sharkY = screenHeight / 2.0f - sharkFrames[0].height / 2.0f;
    float sharkSpeed = 5.0f;  // Shark speed for movement

    // Scaling factor for shark sprite
    float scaleFactor = 4.0f;

    // Animation variables
    int frameCounter = 0;
    int currentFrame = 0;
    int frameSpeed = 8;  // Speed of frame change (lower is faster)

    // Obstacle variables
    Obstacle obstacles[MAX_OBSTACLES];

    // Initialize obstacles with different textures and set hitboxes
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        // Randomize obstacle type, reducing seahorse spawn frequency
        int obstacleType = GetRandomValue(0, 9); // 0-9 gives more control for rarer seahorses
        switch (obstacleType) {
            case 0:
            case 1:
                obstacles[i].texture = sharkNet;
                break;
            case 2:
            case 3:
                obstacles[i].texture = stingray; // Stingray replaces eel
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
                obstacles[i].texture = seahorse; // Seahorse spawns less frequently
                break;
        }
        obstacles[i].position = (Vector2){ screenWidth + i * 200, GetRandomValue(0, screenHeight - obstacles[i].texture.height) };
        obstacles[i].speed = GetRandomValue(4, 8);
        obstacles[i].isActive = true;
        obstacles[i].hitbox = (Rectangle){obstacles[i].position.x + 5, obstacles[i].position.y + 5, obstacles[i].texture.width - 10, obstacles[i].texture.height - 10};  // Shrink hitbox
    }

    // Shark hitbox adjustment for collision accuracy (shrink hitbox)
    Rectangle sharkHitbox = { sharkX + 20, sharkY + 20, sharkFrames[0].width * scaleFactor - 40, sharkFrames[0].height * scaleFactor - 40 };

    // Score and immunity variables
    int score = 0;
    bool isImmune = false;
    float immunityTimer = 0.0f;

    // Background movement
    float backgroundX = 0.0f;
    float backgroundSpeed = 2.0f;

    // Set the target FPS
    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose()) {
        // Update shark position based on key inputs
        if (IsKeyDown(KEY_UP)) sharkY -= sharkSpeed;   // Move up
        if (IsKeyDown(KEY_DOWN)) sharkY += sharkSpeed; // Move down
        if (IsKeyDown(KEY_LEFT)) sharkX -= sharkSpeed; // Move left
        if (IsKeyDown(KEY_RIGHT)) sharkX += sharkSpeed; // Move right

        // Prevent the shark from going off the screen vertically
        if (sharkY < 0) sharkY = 0;
        if (sharkY + sharkFrames[0].height * scaleFactor > screenHeight) 
            sharkY = screenHeight - sharkFrames[0].height * scaleFactor;

        // Prevent the shark from going off the screen horizontally
        if (sharkX < 0) sharkX = 0;
        if (sharkX + sharkFrames[0].width * scaleFactor > screenWidth) 
            sharkX = screenWidth - sharkFrames[0].width * scaleFactor;

        // Background scrolling
        backgroundX -= backgroundSpeed;
        if (backgroundX <= -screenWidth) backgroundX = 0;  // Reset background when fully off screen

        // Frame counter for animation
        frameCounter++;
        if (frameCounter >= (60 / frameSpeed)) {
            frameCounter = 0;
            currentFrame++;
            if (currentFrame >= frameCount) currentFrame = 0;
        }

        // Update shark hitbox based on current position
        sharkHitbox.x = sharkX + 20;
        sharkHitbox.y = sharkY + 20;

        // Update obstacles
        for (int i = 0; i < MAX_OBSTACLES; i++) {
            if (obstacles[i].isActive) {
                obstacles[i].position.x -= obstacles[i].speed;

                // Reset obstacle when off the screen
                if (obstacles[i].position.x + obstacles[i].texture.width < 0) {
                    obstacles[i].position.x = screenWidth + GetRandomValue(0, 200);
                    obstacles[i].position.y = GetRandomValue(0, screenHeight - obstacles[i].texture.height);
                    obstacles[i].speed = GetRandomValue(4, 8);

                    // Randomize obstacle type, reducing seahorse spawn frequency
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

                // Update obstacle hitbox position
                obstacles[i].hitbox.x = obstacles[i].position.x + 5;
                obstacles[i].hitbox.y = obstacles[i].position.y + 5;

                // Check for collision with shark using adjusted hitboxes
                // Check for collision with shark using adjusted hitboxes
              if (CheckCollisionRecs(sharkHitbox, obstacles[i].hitbox)) {
                if (!isImmune) {
                  // Handle collision
                   if (obstacles[i].texture.id == bigFish.id) {
                    score += 10;  // Big fish gives 10 points
                } else if (obstacles[i].texture.id == smallFish.id) {
                    score += 5;   // Small fish gives 5 points
                } else if (obstacles[i].texture.id == sharkNet.id) {
                   // Sharknet causes game over
                DrawText("Game Over!", screenWidth / 2 - 50, screenHeight / 2 - 10, 40, RED);
                EndDrawing();
                sleep(2); // Pause before closing or resetting
                CloseWindow(); // Or reset the game
                return 0; // End the program
                } else if (obstacles[i].texture.id == seahorse.id) {
                // Grant immunity for 3 seconds
                isImmune = true;
                immunityTimer = 3.0f; // 3 seconds of immunity
            } else if (obstacles[i].texture.id == stingray.id) {
                score -= 5;   // Stingray gives -5 points
                if (score < 0) score = 0;  // Prevent score from going below 0
            }
           } else {
                // If immune, only add points for big or small fish
            if (obstacles[i].texture.id == bigFish.id) {
                score += 10;
            } else if (obstacles[i].texture.id == smallFish.id) {
            score += 5;
        }
    }

        // Reset the obstacle position and speed after collision
        obstacles[i].position.x = screenWidth + GetRandomValue(0, 200);
        obstacles[i].position.y = GetRandomValue(0, screenHeight - obstacles[i].texture.height);
        obstacles[i].speed = GetRandomValue(4, 8);
}

            }
        }

        // Update immunity timer
        if (isImmune) {
            immunityTimer -= GetFrameTime();
            if (immunityTimer <= 0.0f) isImmune = false; // Remove immunity when timer runs out
        }

        // Drawing
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw the background
        DrawTexture(orig_big, backgroundX, 0, WHITE);
        DrawTexture(orig_big, backgroundX + screenWidth, 0, WHITE);

        // Draw the shark with scaling
        DrawTextureEx(sharkFrames[currentFrame], (Vector2){sharkX, sharkY}, 0.0f, scaleFactor, WHITE);

        // Draw obstacles
        for (int i = 0; i < MAX_OBSTACLES; i++) {
            if (obstacles[i].isActive) {
                DrawTexture(obstacles[i].texture, (int)obstacles[i].position.x, (int)obstacles[i].position.y, WHITE);
            }
        }

        // Draw the score
        DrawText(TextFormat("Score: %d", score), 10, 10, 20, BLACK);

        if (isImmune) {
            DrawText("Immunity Active!", screenWidth / 2 - 70, screenHeight / 2 - 10, 20, GREEN);
        }

        EndDrawing();
    }

    // De-Initialization
    for (int i = 0; i < frameCount; i++) {
        UnloadTexture(sharkFrames[i]);
    }
    UnloadTexture(orig_big);
    UnloadTexture(sharkNet);
    UnloadTexture(stingray);
    UnloadTexture(smallFish);
    UnloadTexture(bigFish);
    UnloadTexture(seahorse);

    CloseWindow(); // Close window and OpenGL context

    return 0;
}