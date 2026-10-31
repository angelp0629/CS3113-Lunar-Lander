/**
* Author: Angel Perez
* Assignment: Lunar Lander!
* Date due: 10/30/2027
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/Entity.h"
#include <cmath>

// -----------------------------------------------------------------------------
// Global Constants
// -----------------------------------------------------------------------------
constexpr int SCREEN_WIDTH = 320,
              SCREEN_HEIGHT = 180,
              WINDOW_SCALE = 3,
              FPS = 120;

constexpr float FIXED_TIMESTEP = 1.0f / 60.0f,
                ACCELERATION_OF_GRAVITY = 93.0f,
                SIDE_THRUST = 230.0f,
                UP_THRUST = 210.0f,
                HORIZONTAL_DRAG = 4.0f,
                MAX_FUEL = 100.0f,
                FUEL_BURN_PER_KEY = 12.0f;

const Rectangle BUILDING_LEFT   = {0.0f,   150.0f, 76.0f, 30.0f};
const Rectangle BUILDING_CENTER = {112.0f, 143.0f, 43.0f, 37.0f};
const Rectangle BUILDING_RIGHT  = {195.0f, 151.0f, 29.0f, 29.0f};

const Color SKY_TOP    = {15, 18, 49, 255};
const Color SKY_BOTTOM = {48, 44, 91, 255};
const Color OUTLINE    = {22, 19, 42, 255};
const Color CREAM      = {254, 239, 193, 255};
const Color CAT_PINK   = {252, 126, 153, 255};
const Color MINT       = {123, 230, 195, 255};
const Color CAT_GOLD   = {255, 203, 109, 255};

enum AppStatus  { RUNNING, TERMINATED };
enum GameState  { TITLE, PLAYING, WON, LOST };

// Global Variables
AppStatus gAppStatus = RUNNING;
GameState gGameState = TITLE;

float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f,
      gElapsed         = 0.0f,
      gAnimationTime   = 0.0f,
      gFuel            = MAX_FUEL;

bool gLeftPressed  = false,
     gRightPressed = false,
     gUpPressed    = false;

Entity *gProtag     = nullptr;
Entity *gLandingBox = nullptr;

RenderTexture2D gScene = {};
Texture2D gSpikeTexture = {}, gMoonTexture = {}, gCloudTexture = {};
Sound gBoostSound = {}, gWinSound = {}, gFailSound = {};

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

void resetGame();
void updateGameplay(float deltaTime);
bool landedFromAbove(Rectangle before, Rectangle after, Rectangle platform);
void drawBackground();
void drawBuilding(Rectangle box, bool windows);
void drawWorld();
void drawFuelUI(float fuel);
void drawPanel(const char* heading, const char* line1,
               const char* line2, Color headingColor);

bool landedFromAbove(Rectangle before, Rectangle after, Rectangle platform)
{
    const float previousBottom = before.y + before.height;
    const float currentBottom = after.y + after.height;

    return previousBottom <= platform.y + 1.5f &&
           currentBottom >= platform.y &&
           after.x + after.width > platform.x + 5.0f &&
           after.x < platform.x + platform.width - 5.0f;
}

void drawBackground() {
    for (int y = 0; y < SCREEN_HEIGHT; y += 5) {
        const float amount = static_cast<float>(y) / SCREEN_HEIGHT;
        DrawRectangle(0, y, SCREEN_WIDTH, 5, Color{
            static_cast<unsigned char>(SKY_TOP.r + (SKY_BOTTOM.r - SKY_TOP.r) * amount),
            static_cast<unsigned char>(SKY_TOP.g + (SKY_BOTTOM.g - SKY_TOP.g) * amount),
            static_cast<unsigned char>(SKY_TOP.b + (SKY_BOTTOM.b - SKY_TOP.b) * amount), 255});
    }

    for (int i = 0; i < 45; ++i) {
        const int x = (i * 79 + 17) % SCREEN_WIDTH;
        const int y = (i * 47 + 13) % 118;
        const bool flicker = (static_cast<int>(gElapsed * 3.0f) + i) % 7 == 0;
        DrawRectangle(x, y, flicker ? 2 : 1, flicker ? 2 : 1,
                      flicker ? CREAM : Color{152, 156, 194, 255});
    }

    DrawTexture(gMoonTexture, 270, 22, WHITE);
    const int drift = static_cast<int>(std::sin(gElapsed * 0.35f) * 7.0f);
    DrawTexture(gCloudTexture, 157 + drift, 28, WHITE);
    DrawTexture(gCloudTexture, 53 - drift, 55, Color{163, 164, 202, 255});

    for (int i = 0; i < 15; ++i) {
        const int x = i * 24 - 5;
        const int h = 22 + ((i * 13) % 29);
        const int top = 161 - h;
        DrawRectangle(x, top, 21, h, Color{39, 37, 73, 255});
        for (int wy = top + 6; wy < 160; wy += 9) {
            for (int wx = x + 4; wx < x + 19; wx += 8) {
                if ((wx + wy + i) % 3 != 0)
                    DrawRectangle(wx, wy, 2, 3, Color{97, 89, 118, 255});
            }
        }
    }
}

void drawBuilding(Rectangle box, bool windows) {
    DrawRectangleRec(box, OUTLINE);
    DrawRectangle(static_cast<int>(box.x) + 2, static_cast<int>(box.y) + 3,
                  static_cast<int>(box.width) - 4, static_cast<int>(box.height),
                  Color{72, 66, 105, 255});
    DrawRectangle(static_cast<int>(box.x), static_cast<int>(box.y),
                  static_cast<int>(box.width), 4, CAT_PINK);
    DrawRectangle(static_cast<int>(box.x), static_cast<int>(box.y) + 4,
                  static_cast<int>(box.width), 2, OUTLINE);
    if (windows) {
        for (int y = static_cast<int>(box.y) + 11;
             y < static_cast<int>(box.y + box.height) - 3; y += 11) {
            for (int x = static_cast<int>(box.x) + 8;
                 x < static_cast<int>(box.x + box.width) - 5; x += 13) {
                DrawRectangle(x, y, 5, 6, CAT_GOLD);
                DrawRectangle(x + 1, y, 1, 6, OUTLINE);
            }
        }
    }
}

void drawWorld() {
    DrawRectangle(0, 168, SCREEN_WIDTH, 12, OUTLINE);
    for (int x = 0; x < SCREEN_WIDTH; x += 16) {
        DrawTexture(gSpikeTexture, x, 160, WHITE);
    }
    drawBuilding(BUILDING_LEFT, true);
    drawBuilding(BUILDING_CENTER, true);
    drawBuilding(BUILDING_RIGHT, true);
    DrawText("!", 129, 131, 10, CREAM);
}

void drawFuelUI(float fuel) {
    DrawText("FUEL", 9, 10, 10, CREAM);
    DrawRectangle(38, 10, 83, 12, OUTLINE);
    DrawRectangle(40, 12, 79, 8, Color{57, 55, 86, 255});
    const int barWidth = static_cast<int>((fuel / MAX_FUEL) * 79.0f);
    const Color barColor = fuel < 25.0f ? CAT_PINK : MINT;
    DrawRectangle(40, 12, barWidth, 8, barColor);
    DrawRectangleLines(39, 11, 81, 10, CREAM);
    DrawText("A/D MOVE  W/UP BOOST", 9, 30, 8, CREAM);
}

void drawPanel(const char* heading, const char* line1, const char* line2,
               Color headingColor) {
    DrawRectangle(27, 45, 268, 93, Color{15, 17, 37, 237});
    DrawRectangleLines(27, 45, 268, 93, CREAM);
    DrawRectangleLines(30, 48, 262, 87, Color{114, 112, 151, 255});
    const int headingSize = 16;
    const int titleX = (SCREEN_WIDTH - MeasureText(heading, headingSize)) / 2;
    DrawText(heading, titleX, 60, headingSize, headingColor);
    DrawText(line1, (SCREEN_WIDTH - MeasureText(line1, 10)) / 2, 92, 10, CREAM);
    DrawText(line2, (SCREEN_WIDTH - MeasureText(line2, 9)) / 2, 113, 9,
             Color{181, 183, 213, 255});
}


void resetGame()
{
    gProtag->reset({42.0f, 32.0f});
    gProtag->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

    gLandingBox->reset({244.0f, 140.0f});
    gLandingBox->setVelocity({-15.0f, 0.0f});

    gFuel = MAX_FUEL;
    gAnimationTime = 0.0f;
    gTimeAccumulator = 0.0f;
    gGameState = PLAYING;
}

void initialise()
{
    InitWindow(SCREEN_WIDTH * WINDOW_SCALE, SCREEN_HEIGHT * WINDOW_SCALE,
               "Cat-astrophic Landing | 8-Bit Edition");
    InitAudioDevice();

    gScene = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    SetTextureFilter(gScene.texture, TEXTURE_FILTER_POINT);

    gSpikeTexture = LoadTexture("assets/spikes.png");
    gMoonTexture = LoadTexture("assets/moon.png");
    gCloudTexture = LoadTexture("assets/cloud.png");

    gBoostSound = LoadSound("assets/boost.wav");
    gWinSound = LoadSound("assets/win.wav");
    gFailSound = LoadSound("assets/fail.wav");

    // The Entity object owns and renders its own sprite sheet.
    // Four 24x28 frames are arranged in one horizontal row.
    gProtag = new Entity({42.0f, 32.0f}, {24.0f, 28.0f},
                         "assets/cat_jetpack.png", 4, 1);
    gProtag->setColliderDimensions({20.0f, 21.0f});
    gProtag->setRenderOffset({-2.0f, -3.0f});
    gProtag->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

    // The moving landing platform is an Entity as well.
    gLandingBox = new Entity({244.0f, 140.0f}, {48.0f, 13.0f},
                             "assets/cardboard_box.png");
    gLandingBox->setColliderDimensions({48.0f, 13.0f});
    gLandingBox->setVelocity({-15.0f, 0.0f});

    gPreviousTicks = static_cast<float>(GetTime());
    SetTargetFPS(FPS);
}

void processInput()
{
    if (IsKeyPressed(KEY_Q) || WindowShouldClose())
    {
        gAppStatus = TERMINATED;
        return;
    }

    if (gGameState == TITLE &&
        (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)))
        resetGame();
    else if (gGameState != TITLE && IsKeyPressed(KEY_R))
        resetGame();

    gLeftPressed = IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT);
    gRightPressed = IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT);
    gUpPressed = IsKeyDown(KEY_W) || IsKeyDown(KEY_UP) || IsKeyDown(KEY_SPACE);

    if (gGameState == PLAYING && gFuel > 0.0f &&
        (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_SPACE)))
        PlaySound(gBoostSound);
}

void updateGameplay(float deltaTime)
{
    Rectangle previousBounds = gProtag->getBounds();
    Vector2 previousVelocity = gProtag->getVelocity();
    const float previousBoxX = gLandingBox->getPosition().x;

    // The cardboard box moves automatically, bouncing between two limits.
    gLandingBox->update(deltaTime);
    Vector2 boxPosition = gLandingBox->getPosition();
    Vector2 boxVelocity = gLandingBox->getVelocity();
    if (boxPosition.x < 224.0f)
    {
        boxPosition.x = 224.0f;
        boxVelocity.x = std::fabs(boxVelocity.x);
    }
    else if (boxPosition.x > 266.0f)
    {
        boxPosition.x = 266.0f;
        boxVelocity.x = -std::fabs(boxVelocity.x);
    }
    gLandingBox->setPosition(boxPosition);
    gLandingBox->setVelocity(boxVelocity);

    // Only change the PLAYER'S acceleration in response to input.
    // This preserves momentum after the keys are released.
    Vector2 acceleration = {0.0f, ACCELERATION_OF_GRAVITY};
    int activeThrustDirections = 0;
    const int axis = static_cast<int>(gRightPressed) -
                     static_cast<int>(gLeftPressed);

    if (axis != 0 && gFuel > 0.0f)
    {
        acceleration.x = axis * SIDE_THRUST;
        activeThrustDirections++;
    }
    else
    {
        // Drag is an acceleration opposite the current horizontal velocity.
        acceleration.x = -previousVelocity.x * HORIZONTAL_DRAG;
    }

    if (gUpPressed && gFuel > 0.0f)
    {
        acceleration.y -= UP_THRUST;
        activeThrustDirections++;
    }

    gFuel -= activeThrustDirections * FUEL_BURN_PER_KEY * deltaTime;
    if (gFuel < 0.0f) gFuel = 0.0f;

    gProtag->setAcceleration(acceleration);
    gProtag->update(deltaTime);
    gAnimationTime += deltaTime;

    // Keep the player inside the left, right, and top of the game area.
    // Only world boundary collisions explicitly clear velocity.
    Vector2 position = gProtag->getPosition();
    Vector2 velocity = gProtag->getVelocity();
    if (position.x < 0.0f)
    {
        position.x = 0.0f;
        velocity.x = 0.0f;
    }
    else if (position.x + 20.0f > SCREEN_WIDTH)
    {
        position.x = SCREEN_WIDTH - 20.0f;
        velocity.x = 0.0f;
    }
    if (position.y < 2.0f)
    {
        position.y = 2.0f;
        velocity.y = 0.0f;
    }
    gProtag->setPosition(position);
    gProtag->setVelocity(velocity);

    const Rectangle catBounds = gProtag->getBounds();
    const Rectangle boxBounds = gLandingBox->getBounds();

    // Touching the moving box from the top at a gentle speed is a win.
    if (CheckCollisionRecs(catBounds, boxBounds))
    {
        const float boxSpeed = (boxPosition.x - previousBoxX) / deltaTime;
        const bool gentleLanding = landedFromAbove(previousBounds, catBounds, boxBounds) &&
                                   previousVelocity.y >= -3.0f &&
                                   velocity.y <= 24.0f &&
                                   std::fabs(velocity.x - boxSpeed) <= 36.0f;
        gGameState = gentleLanding ? WON : LOST;
        PlaySound(gentleLanding ? gWinSound : gFailSound);
    }

    // Buildings and the spikes on the street are forbidden areas.
    if (gGameState == PLAYING &&
        (CheckCollisionRecs(catBounds, BUILDING_LEFT) ||
         CheckCollisionRecs(catBounds, BUILDING_CENTER) ||
         CheckCollisionRecs(catBounds, BUILDING_RIGHT) ||
         catBounds.y + catBounds.height >= 161.0f))
    {
        gGameState = LOST;
        PlaySound(gFailSound);
    }
}

void update()
{
    const float ticks = static_cast<float>(GetTime());
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;

    // Avoid a massive jump after the window loses focus.
    if (deltaTime > 0.1f) deltaTime = 0.1f;
    gElapsed += deltaTime;

    if (gGameState != PLAYING)
    {
        gTimeAccumulator = 0.0f;
        return;
    }

    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP && gGameState == PLAYING)
    {
        updateGameplay(FIXED_TIMESTEP);
        deltaTime -= FIXED_TIMESTEP;
    }

    gTimeAccumulator = gGameState == PLAYING ? deltaTime : 0.0f;
}

void render()
{
    BeginTextureMode(gScene);
    ClearBackground(SKY_TOP);

    drawBackground();
    drawWorld();

    gLandingBox->render();
    DrawText("LAND", static_cast<int>(gLandingBox->getPosition().x) + 13,
             static_cast<int>(gLandingBox->getPosition().y) - 12, 9, MINT);
    DrawLine(static_cast<int>(gLandingBox->getPosition().x) + 23,
             static_cast<int>(gLandingBox->getPosition().y) - 3,
             static_cast<int>(gLandingBox->getPosition().x) + 23,
             static_cast<int>(gLandingBox->getPosition().y) - 1, MINT);

    const bool jetpackOn = gGameState == PLAYING && gFuel > 0.0f && gUpPressed;
    const int frame = jetpackOn
        ? 1 + (static_cast<int>(gAnimationTime * 10.0f) % 3)
        : 0;
    gProtag->setFrame(frame);
    gProtag->render();

    drawFuelUI(gFuel);
    if (gGameState == TITLE)
        drawPanel("CAT-ASTROPHIC", "LAND ON THE MOVING BOX!",
                  "ENTER TO START | A/D + W/UP", CAT_GOLD);
    else if (gGameState == WON)
        drawPanel("MISSION ACCOMPLISHED", "PURR-FECT LANDING! NICE PAWS.",
                  "PRESS R TO PLAY AGAIN", MINT);
    else if (gGameState == LOST)
        drawPanel("MISSION FAILED", "THAT LANDING WAS A CAT-ASTROPHE!",
                  "PRESS R TO TRY AGAIN", CAT_PINK);
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexturePro(gScene.texture,
                   {0.0f, 0.0f, static_cast<float>(SCREEN_WIDTH),
                    -static_cast<float>(SCREEN_HEIGHT)},
                   {0.0f, 0.0f, static_cast<float>(SCREEN_WIDTH * WINDOW_SCALE),
                    static_cast<float>(SCREEN_HEIGHT * WINDOW_SCALE)},
                   {0.0f, 0.0f}, 0.0f, WHITE);
    EndDrawing();
}

void shutdown()
{
    delete gProtag;
    delete gLandingBox;

    UnloadSound(gFailSound);
    UnloadSound(gWinSound);
    UnloadSound(gBoostSound);

    UnloadTexture(gCloudTexture);
    UnloadTexture(gMoonTexture);
    UnloadTexture(gSpikeTexture);
    UnloadRenderTexture(gScene);

    CloseAudioDevice();
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        if (gAppStatus == TERMINATED) break;
        update();
        render();
    }

    shutdown();
    return 0;
}
