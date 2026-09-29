#include <float.h>
#include <math.h>
#include <stdio.h>
#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include <raylib-nuklear.h>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#define MAX_SPLINE_POINTS      320000

typedef struct {
    double x;
    double y;
} Vec2;

typedef struct {
    Vec2 pos;
    double mass;
    Vec2 velocity;  // 10^0 = 1m/s
    double radius;  // 10^0 = 1 meter
} Object;

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

//Calc the magnitude
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
           double posy)
{
    Object obj = { .mass     = mass,
                   .pos      = { .x = posx, .y = posy },
                   .radius   = radius,
                   .velocity = { .x = velx, .y = vely } };

    return obj;
}

int
main()
{
    Vec2 worldSpace = { .x = 0.0, .y = 0.0 };
    const double G  = 6.6743e-11;

    Vec2 curvePoints[MAX_SPLINE_POINTS];
    int trailHead  = 0;
    int trailCount = 0;

    Object bodies[3];

    // bodies[0] = makeObject(5.9722e24, 6.371e6, -2.9785e4, 0.0, 0.0, 0.0);
    // bodies[1] = makeObject(7.3470e22, 1.740e6, -1.022e3, 0.0, 0.0, 3.844e8);
    // bodies[2] = makeObject(1.98892e30, 6.957e8, -2.30e5, 0.0, 0.0,
    //                        1.495978707e11);
    bodies[0] = makeObject(5.9722e24, 6.371e6, -2.9785e3, 0.0, 0.0, 0.0);
    bodies[1] = makeObject(7.3470e22, 1.740e6, -1.022e3, 0.0, 0.0, 3.844e8);
    bodies[2] = makeObject(1.98892e30, 6.957e8, 0.0, 0.0, 0.0, 1.495978707e11);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(1920, 1080, "N-body gravity sim");

    Camera2D camera = { .zoom     = 1.0f,
                        .offset   = { .x = 0.0f, .y = 0.0f },
                        .rotation = 0.0f,
                        .target   = { .x = 0.0f, .y = 0.0f } };
    // Create the Nuklear Context
    int fontSize           = 18;
    struct nk_context* ctx = InitNuklear(fontSize);

    SetTargetFPS(60);
    // char *FPSstring = NULL;
    double timeScale = 0.1;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        for (size_t i = 0; i < sizeof(bodies) / sizeof(Object); i++) {
            Vec2 cumulativeForce = { .x = 0, .y = 0 };
            for (size_t j = 0; j < sizeof(bodies) / sizeof(Object); j++) {
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
                                         Vec2Scale(acc_i, dt * timeScale));

            bodies[i].pos = Vec2Add(bodies[i].pos, Vec2Scale(bodies[i].velocity,
                                                             dt * timeScale));
            if (i == 0) {
                curvePoints[trailHead] = bodies[i].pos;

                trailHead = (trailHead + 1) % MAX_SPLINE_POINTS;

                if (trailCount < MAX_SPLINE_POINTS)
                    trailCount++;
            }
        }
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

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Vec2 delta = Vec2ScaleF(GetMouseDelta(), -1.0 / camera.zoom);

            worldSpace      = Vec2Add(worldSpace, delta);
            camera.target.x = worldSpace.x;
            camera.target.y = worldSpace.y;
        }
        if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
            worldSpace = bodies[0].pos;
        }
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(),
                                                       camera);
            camera.offset         = GetMousePosition();

            worldSpace.x  = mouseWorldPos.x;
            worldSpace.y  = mouseWorldPos.y;
            camera.target = mouseWorldPos;

            float scale = 0.2f * wheel;
            camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.000000000005,
                                64.0f);
        }
        UpdateNuklear(ctx);

        if (nk_begin(ctx, "Nuklear", nk_rect(0, 0, 420, GetScreenHeight()),
                     NK_WINDOW_BORDER | NK_WINDOW_MOVABLE
                         | NK_WINDOW_CLOSABLE)) {
            nk_layout_row_static(ctx, 50, 400, 1);
            nk_label(ctx, "Time Scale", NK_TEXT_CENTERED);
            // timeScale = nk_slide_float(ctx, 0.01f, timeScale, 200.0f, 5.0f);
            nk_property_double(ctx, "Time scale", 0.0001, &timeScale, DBL_MAX,
                               0.5, 1.0);
            char velocity_str[54];
            char distance_sun[64];
            sprintf(velocity_str, "Earth's velocity: %f m/s",
                    Vec2Length(bodies[0].velocity));
            sprintf(distance_sun, "Earth's distance to sun: %f km",
                    Vec2Distance(bodies[0].pos, bodies[2].pos) * 1000);
            nk_label(ctx, velocity_str, NK_TEXT_ALIGN_LEFT);
            nk_label(ctx, distance_sun, NK_TEXT_ALIGN_LEFT);
            // nk_property_double(ctx, "planet 1 mass", DBL_MIN, &mass1, DBL_MAX,
            //                    0.5, 1.0);
            // nk_property_double(ctx, "planet 2 mass", DBL_MIN, &mass2, DBL_MAX,
            //                    0.5, 1.0);
            if (nk_button_label(ctx, "Button")) {
                // Button was clicked!
            }
        }
        nk_end(ctx);

        // Render
        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode2D(camera);
        rlPushMatrix();
        rlTranslatef(0, 20 * 20, 0);
        rlRotatef(90, 1, 0, 0);
        DrawGrid(10000 * camera.zoom, 50 * (1.0f / camera.zoom));

        rlPopMatrix();
        for (size_t i = 0; i < (sizeof(bodies) / sizeof(Object)); i++) {
            DrawCircleV(ToRenderPos(bodies[i].pos, worldSpace),
                        bodies[i].radius, YELLOW);
        }
        // DrawLineBezier(ToRenderPos(initial, worldSpace),
        //                ToRenderPos(pos1, worldSpace), 9000.0f, RED);
        for (int i = 1; i < trailCount; i++) {
            int prev = (trailHead - trailCount + i - 1 + MAX_SPLINE_POINTS)
                       % MAX_SPLINE_POINTS;

            int curr = (trailHead - trailCount + i + MAX_SPLINE_POINTS)
                       % MAX_SPLINE_POINTS;

            DrawLineV(ToRenderPos(curvePoints[prev], worldSpace),
                      ToRenderPos(curvePoints[curr], worldSpace), WHITE);
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
    return 0;
}
