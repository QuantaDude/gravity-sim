#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include <raylib-nuklear.h>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#define ARENA_IMPLEMENTATION
#include "mem_arena.h"

#define MAX_SPLINE_POINTS 1000

typedef struct {
    double x;
    double y;
} Vec2;

typedef struct {
    char* name;
    Vec2 pos;
    double mass;
    Vec2 velocity;  // 10^0 = 1m/s
    double radius;  // 10^0 = 1 meter

    Vec2 curvePoints[MAX_SPLINE_POINTS];
    int trailHead;
    int trailCount;
} Object;

#pragma region helpers
Vec2
Vec2Scale(Vec2 v, double scale)
{
    Vec2 result = { v.x * scale, v.y * scale };
    return result;
}

Vec2
Vec2ScaleF(Vector2 v, double scale)
{
    Vec2 result = { v.x * scale, v.y * scale };
    return result;
}

Vec2
Vec2Add(Vec2 v1, Vec2 v2)
{
    Vec2 result = { v1.x + v2.x, v1.y + v2.y };
    return result;
}

Vec2
Vec2Sub(Vec2 v1, Vec2 v2)
{
    Vec2 result = { v1.x - v2.x, v1.y - v2.y };
    return result;
}

double
Vec2Distance(Vec2 v1, Vec2 v2)
{
    double result = sqrt((v1.x - v2.x) * (v1.x - v2.x)
                         + (v1.y - v2.y) * (v1.y - v2.y));
    return result;
}

// Calc the magnitude
double
Vec2Length(Vec2 v)
{
    double result = sqrt((v.x * v.x) + (v.y * v.y));

    return result;
}

double
Vec2LengthSqr(Vec2 v)
{
    double result = (v.x * v.x) + (v.y * v.y);

    return result;
}

Vector2
ToRenderPos(Vec2 pos, Vec2 origin)
{
    Vector2 result = { .x = (float)(pos.x - origin.x),
                       .y = (float)(pos.y - origin.y) };
    return result;
}

Object
makeObject(double mass, double radius, double velx, double vely, double posx,
           double posy, const char* name)
{
    Object obj = { .mass        = mass,
                   .pos         = { .x = posx, .y = posy },
                   .radius      = radius,
                   .velocity    = { .x = velx, .y = vely },
                   .trailHead   = 0,
                   .trailCount  = 0,
                   .curvePoints = {} };

    obj.name = malloc(sizeof(name));
    strcpy(obj.name, name);

    return obj;
}
void
initObject(Object* obj, double mass, double radius, double velx, double vely,
           double posx, double posy, const char* name)
{
    obj->pos        = (Vec2) { posx, posy };
    obj->mass       = mass;
    obj->velocity   = (Vec2) { velx, vely };
    obj->radius     = radius;
    obj->trailHead  = 0;
    obj->trailCount = 0;

    obj->name = calloc(strlen(name), sizeof(char));
    strcpy(obj->name, name);
}
#pragma endregion

typedef struct {
    Shader planet_shader;
    struct nk_context* ctx;
    Vec2 worldSpace;
    double time_scale;
    float time;
    Object* bodies;

} App;

int
main()
{
#pragma region init
    void* mem = malloc(289152);
    Arena a   = { 0 };
    arena_init(&a, mem, 289152);
    App* sim              = (App*)arena_alloc(&a, sizeof(App));
    sim->worldSpace.x     = 0.0;
    sim->worldSpace.y     = 0.0;
    sim->time_scale       = 0.1;
    sim->time             = (float)GetTime();
    uint32_t selected_obj = 0;
    const double G        = 6.6743e-11;

    float* time       = &sim->time;
    double* timeScale = &sim->time_scale;

#pragma region alloc_init_inital_bodies
    Object* bodies = calloc(6, sizeof(Object));
    if (!bodies) {
        fprintf(stderr, "Failed to allocate bodies\n");
        return 1;
    }

    initObject(&bodies[0], 5.9722e24, 6.371e6, -2.9785e4, 0.0, 0.0, 0.0,
               "Earth");

    initObject(&bodies[1], 7.3470e22, 1.740e6, -1.022e5, 0.0, 0.0, 3.844e8,
               "Moon");

    initObject(&bodies[2], 1.98892e30, 6.957e8, 1.022e4, -1.021321e4, 0.0,
               8.495978707e10, "Sun 1");

    initObject(&bodies[3], 1.898e27, 7.1492e7, -1.306e4, 0.0, 0.0, -7.78e11,
               "Jupiter 1");
    initObject(&bodies[4], 1.898e27, 7.1492e7, -1.306e4, 0.0, 0.0, -7.78e9,
               "Jupiter 2");
    initObject(&bodies[5], 1.98892e31, 6.957e9, 0.0, 0.0, 0.0, -1.795978707e11,
               "Mega Sun");
#pragma endregion

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(1920, 1080, "N-body gravity sim");
    SetTargetFPS(60);

    Camera2D camera = { .zoom     = 1.0f,
                        .offset   = { .x = 0.0f, .y = 0.0f },
                        .rotation = 0.0f,
                        .target   = { .x = 0.0f, .y = 0.0f } };

    // Create the Nuklear Context
    int fontSize           = 18;
    struct nk_context* ctx = InitNuklear(fontSize);

#pragma region setup_shaders
    Shader planetShader = LoadShader(0, "./shaders/planet_1.glsl");
    int centerLoc       = GetShaderLocation(planetShader, "uCircleCenter");
    int radiusLoc       = GetShaderLocation(planetShader, "uCircleRadius");
    int seedLoc         = GetShaderLocation(planetShader, "u_seed");
    int seed            = 1337;
    SetShaderValue(planetShader, seedLoc, &seed, SHADER_UNIFORM_UINT);
#pragma endregion

#pragma endregion init

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        *time    = (float)(*time + (dt * *timeScale));
        SetShaderValue(planetShader, GetShaderLocation(planetShader, "uTime"),
                       time, SHADER_UNIFORM_FLOAT);



#pragma region calc_forces_and_curve_points
        for (size_t i = 0; i < 6; i++) {
            Vec2 cumulativeForce = { .x = 0, .y = 0 };
            for (size_t j = 0; j < 6; j++) {
                if (i == j)
                    continue;
                Vec2 displacement = Vec2Sub(bodies[i].pos, bodies[j].pos);
                double displaceMagnitudeSq = Vec2LengthSqr(displacement);

                double forceMagnitude = (G * bodies[i].mass * bodies[j].mass)
                                        / displaceMagnitudeSq;
                Vec2 direction  = Vec2Scale(displacement,
                                            1.0f / Vec2Length(displacement));
                Vec2 force_ji   = Vec2Scale(direction, -forceMagnitude);
                cumulativeForce = Vec2Add(cumulativeForce, force_ji);
            }
            Vec2 acc_i = Vec2Scale(cumulativeForce, 1.0 / bodies[i].mass);
            bodies[i].velocity = Vec2Add(bodies[i].velocity,
                                         Vec2Scale(acc_i, dt * *timeScale));

            bodies[i].pos = Vec2Add(bodies[i].pos, Vec2Scale(bodies[i].velocity,
                                                             dt * *timeScale));
            bodies[i].curvePoints[bodies[i].trailHead] = bodies[i].pos;

            bodies[i].trailHead = (bodies[i].trailHead + 1) % MAX_SPLINE_POINTS;

            if (bodies[i].trailCount < MAX_SPLINE_POINTS)
                bodies[i].trailCount++;
        }
#pragma endregion
#pragma region old_gravity_code
        // delta
        // Vec2 displacementVect      = Vec2Sub(pos1, pos2);
        // double displaceMagnitudeSq = Vec2LengthSqr(displacementVect);
        //
        // double forceMagnitude = (G * mass1 * mass2) / displaceMagnitudeSq;
        //
        // Vec2 direction = Vec2Scale(displacementVect,
        //                            1.0f / Vec2Length(displacementVect));
        //
        // Vec2 F21 = Vec2Scale(direction, -forceMagnitude);
        //
        // Vec2 a1 = Vec2Scale(F21, 1.0f / mass1);
        // Vec2 a2 = Vec2Scale(F21, -1.0f / mass2);
        //
        //
        // vel1 = Vec2Add(vel1, Vec2Scale(a1, dt * timeScale));
        // vel2 = Vec2Add(vel2, Vec2Scale(a2, dt * timeScale));
        //
        // pos2 = Vec2Add(pos2, Vec2Scale(vel2, dt * timeScale));
        // pos1 = Vec2Add(pos1, Vec2Scale(vel1, dt * timeScale));

        // curvePoints[trailHead] = pos1;
        //
        // trailHead = (trailHead + 1) % MAX_SPLINE_POINTS;
        //
        // if (trailCount < MAX_SPLINE_POINTS)
        //     trailCount++;
#pragma endregion
#pragma region camera_controls
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Vec2 delta = Vec2ScaleF(GetMouseDelta(), -1.0 / camera.zoom);

            sim->worldSpace = Vec2Add(sim->worldSpace, delta);
            camera.target.x = sim->worldSpace.x;
            camera.target.y = sim->worldSpace.y;
        }
        if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
            sim->worldSpace = bodies[selected_obj].pos;
        }
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(),
                                                       camera);
            camera.offset         = GetMousePosition();

            sim->worldSpace.x = mouseWorldPos.x;
            sim->worldSpace.y = mouseWorldPos.y;
            camera.target     = mouseWorldPos;

            float scale = 0.2f * wheel;
            camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.000000000005,
                                64.0f);
        }
#pragma endregion
#pragma region draw_ui
        UpdateNuklear(ctx);

        if (nk_begin(ctx, "Gravity-sim", nk_rect(0, 0, 420, GetScreenHeight()),
                     NK_WINDOW_BORDER | NK_WINDOW_MOVABLE
                         | NK_WINDOW_CLOSABLE)) {
            nk_layout_row_static(ctx, 50, 400, 1);
            nk_label(ctx, "Time Scale", NK_TEXT_CENTERED);
            // timeScale = nk_slide_float(ctx, 0.01f, timeScale, 200.0f, 5.0f);
            nk_property_double(ctx, "Time scale", 0.0001, timeScale, DBL_MAX,
                               0.5, 1.0);
            char velocity_str[54];
            char distance_sun[64];
            sprintf(velocity_str, "%s's velocity: %f m/s",
                    bodies[selected_obj].name,
                    Vec2Length(bodies[selected_obj].velocity));
            sprintf(distance_sun, "Earth's distance to %s: %f km",
                    bodies[selected_obj].name,
                    Vec2Distance(bodies[0].pos, bodies[selected_obj].pos)
                        * 1000);
            nk_label(ctx, velocity_str, NK_TEXT_ALIGN_LEFT);
            nk_label(ctx, distance_sun, NK_TEXT_ALIGN_LEFT);
            // nk_property_double(ctx, "planet 1 mass", DBL_MIN, &mass1, DBL_MAX,
            //                    0.5, 1.0);
            // nk_property_double(ctx, "planet 2 mass", DBL_MIN, &mass2, DBL_MAX,
            //                    0.5, 1.0);
            if (nk_button_label(ctx, "go to body")) {
                sim->worldSpace = bodies[selected_obj].pos;
            }
        }
        nk_end(ctx);
#pragma endregion
        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode2D(camera);
        rlPushMatrix();
        rlTranslatef(0, 20 * 20, 0);
        rlRotatef(90, 1, 0, 0);
        DrawGrid(10000 * camera.zoom, 50 * (1.0f / camera.zoom));

        rlPopMatrix();
        Vector2 res = { (float)GetScreenWidth(), (float)GetScreenHeight() };

        for (size_t i = 0; i < 6; i++) {
            Vector2 renderPos = ToRenderPos(bodies[i].pos, sim->worldSpace);
            Vector2 screenPos = GetWorldToScreen2D(renderPos, camera);
            Vector2 center    = { screenPos.x, res.y - screenPos.y };
            float radius      = (float)bodies[i].radius * camera.zoom;

            SetShaderValue(planetShader, centerLoc, &center,
                           SHADER_UNIFORM_VEC2);
            SetShaderValue(planetShader, radiusLoc, &radius,
                           SHADER_UNIFORM_FLOAT);
            BeginShaderMode(planetShader);
            DrawCircleV(ToRenderPos(bodies[i].pos, sim->worldSpace),
                        bodies[i].radius, YELLOW);
            EndShaderMode();

            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                if (CheckCollisionPointCircle(
                        GetScreenToWorld2D(GetMousePosition(), camera),
                        renderPos, bodies[i].radius)) {
                    selected_obj = i;
                }
            }
        }

        for (int body = 0; body < 6; body++) {
            for (int i = 1; i < bodies[body].trailCount; i++) {
                int prev = (bodies[body].trailHead - bodies[body].trailCount + i
                            - 1 + MAX_SPLINE_POINTS)
                           % MAX_SPLINE_POINTS;

                int curr = (bodies[body].trailHead - bodies[body].trailCount + i
                            + MAX_SPLINE_POINTS)
                           % MAX_SPLINE_POINTS;

                DrawLineV(ToRenderPos(bodies[body].curvePoints[prev],
                                      sim->worldSpace),
                          ToRenderPos(bodies[body].curvePoints[curr],
                                      sim->worldSpace),
                          WHITE);
            }
        }
        // DrawText("Earth", pos1.x - worldSpace.x - 1000.0f,
        //          (pos1.y - worldSpace.y), 18.0f * (1.0 / camera.zoom), YELLOW);
        //
        // DrawText("Moon", pos2.x - worldSpace.x - 1000.0f,
        //          (pos2.y - worldSpace.y) - (radius2 * 4),
        //          18.0f * (1.0 / camera.zoom), YELLOW);
        EndMode2D();
        // Render the Nuklear GUI
        DrawNuklear(ctx);

        EndDrawing();
    }

    // De-initialize the Nuklear GUI
    UnloadNuklear(ctx);

    CloseWindow();
    free(bodies);
    return 0;
}
