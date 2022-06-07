#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"

#define V3(x, y, z) ((Vector3){x, y, z})

void draw_cross(Vector3 center, int lines_no, float unit_size, Color grid_color) {
    DrawLine3D(V3(center.x - (lines_no / 2) * unit_size, center.y, center.z), V3(center.x + (lines_no / 2) * unit_size, center.y, center.z), BLACK);
    for (int i= (-(lines_no/2)) + 1; i<=(lines_no/2) - 1; i++) {
        DrawLine3D(V3(center.x + (i*unit_size), center.y, center.z - (unit_size / 2)), V3(center.x + (i*unit_size), center.y, center.z + (unit_size / 2)), BLACK);
    }
    DrawLine3D(V3(center.x, center.y, center.z - (lines_no / 2) * unit_size), V3(center.x, center.y, center.z + (lines_no / 2) * unit_size), BLACK);
    for (int i= (-(lines_no/2)) + 1; i<=(lines_no/2) - 1; i++) {
        DrawLine3D(V3(center.x - (unit_size / 2), center.y, center.z + (i*unit_size)), V3(center.x + (unit_size / 2), center.y, center.z + (i*unit_size)), BLACK);
    }
}

void draw_grid(Vector3 center, int lines_no, float unit_size, Color grid_color) {
    for (int i = -(lines_no/2); i<= lines_no/2; i++) {
        DrawLine3D(V3(center.x + (i*unit_size), center.y, center.z-((lines_no/2) * unit_size)), V3(center.x + (i*unit_size), center.y, center.z+((lines_no/2) * unit_size)), grid_color);
        DrawLine3D(V3(center.x-((lines_no/2) * unit_size), center.y, center.z + (i*unit_size)), V3(center.x+((lines_no/2) * unit_size), center.y, center.z + (i*unit_size)), grid_color);
    }
}

void draw_y_axis(Vector3 center, int lines_no, float unit_size) {
    DrawLine3D(V3(center.x, center.y - ((lines_no/2) * unit_size), center.z), V3(center.x, center.y + ((lines_no/2) * unit_size), center.z), BLACK);
    for (int i= -(lines_no/2); i<=lines_no/2; i++) {
        DrawLine3D(V3(center.x - (unit_size / 2), center.y + (i * unit_size), center.z), V3(center.x + (unit_size / 2), center.y + (i * unit_size), center.z), BLACK);
        DrawLine3D(V3(center.x, center.y + (i * unit_size), center.z - (unit_size / 2)), V3(center.x, center.y + (i * unit_size), center.z + (unit_size / 2)), BLACK);
    }
}

//all_points[current_point++] = click_point;
bool check_for_point(Vector3* point_list, int list_size, Vector3 point) {
    for (int i=0; i<list_size; i++) {
        if (point_list[i].x == point.x && point_list[i].y == point.y && point_list[i].z == point.z) {
            return true;
        }
    }
    return false;
}

typedef struct tri {
    int p1, p2, p3;
} tri;

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    static const int SCREEN_WIDTH = 800;
    static const int SCREEN_HEIGHT = 450;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "planes");

    // Define the camera to look into our 3d world
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 10.0f, 10.0f };  // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
    SetCameraMode(camera, CAMERA_FREE);

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    Color grid_color = Fade(BLACK, 0.2);
    Color face_color = Fade(BLUE, 0.5);
    Vector3 center = V3(0, 0, 0);
    RayCollision collision = { 0 };
    Ray ray;
    Vector3 click_point = V3(0, 0, 0);
    Vector3 click_point_absolute = V3(0, 0, 0);
    bool click_point_exists = false;

    static float grid_interval = 1.5f;
    static int lines_no = 8;

    char x_coord[6];
    char z_coord[6];
    char grid_interval_val[4];
    char lines_no_val[2];

    bool point_select = true;
    bool grid_select = false;
    bool point_create = false;
    int current_mode = 1;

    static float current_grid_y = 0;

    Vector3 all_points[100];
    int current_point = 0;
    //int selected_points[100];
    tri faces[100];
    int faces_no = 0;
    int selected_points[100];
    int selected_points_no = 0;

    int y_axis_multiply = 1;

    bool three_points_selected = false;

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera);
        
        if (IsKeyDown('Z')) camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
        if (IsKeyPressed(KEY_W)) grid_interval += 0.25f;
        if (IsKeyPressed(KEY_S)) grid_interval -= 0.25f;
        if (IsKeyPressed(KEY_A)) lines_no += 2;
        if (IsKeyPressed(KEY_D)) lines_no -= 2;
        //if (IsKeyPressed(KEY_LEFT)) current_mode = 3;

        switch (current_mode) {
            case 1:
            {
                if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) current_mode = 2;

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Ray mouse_ray = GetMouseRay(GetMousePosition(), camera);
                    bool hit_point = false;
                    for (int i=0; i<current_point; i++) {
                        collision = GetRayCollisionSphere(mouse_ray, all_points[i], 0.25);
                        bool duplicate = false;
                        if (collision.hit) {
                            hit_point = true;
                            //selected_point = i;
                            for (int j=0; j<selected_points_no; j++) { // avoid putting in the same point into the selected points array twice
                                if (selected_points[j] == i) {
                                    duplicate = true;
                                    //continue;
                                }
                            }
                            if (duplicate) {
                                continue;
                            }
                            selected_points[selected_points_no++] = i;
                            break; // if the mouse ray hits 2 different points, the user deserves to get fucked over
                        }
                    }
                    if (!hit_point) {
                        // instead of actually emptying array or something, just limit access by reseting number
                        selected_points_no = 0;
                    }
                }

                if (IsKeyPressed(KEY_X)) {
                    if (selected_points_no == 3) {
                        faces[faces_no++] = (tri) {selected_points[0], selected_points[1], selected_points[2]};
                        selected_points_no = 0;
                    }
                }

                if (selected_points_no == 1) {
                    if (IsKeyPressed(KEY_UP)) {
                        all_points[selected_points[0]].y += grid_interval / 2;
                    }
                    if (IsKeyPressed(KEY_DOWN)) {
                        all_points[selected_points[0]].y -= grid_interval / 2;
                    }
                    if (IsKeyPressed(KEY_LEFT)) {
                        all_points[selected_points[0]].x -= grid_interval / 2;
                    }
                    if (IsKeyPressed(KEY_RIGHT)) {
                        all_points[selected_points[0]].x += grid_interval / 2;
                    }
                }

            } break;
            case 2:
            {
                //collision = 
                //CheckCollisionRaySphereEx(GetMouseRay(GetMousePosition(), camera), center, grid_interval / 2, collision);
                collision = GetRayCollisionBox(GetMouseRay(GetMousePosition(), camera), (BoundingBox) {V3(center.x - (grid_interval/(lines_no/2)), center.y - ((lines_no/2) * grid_interval * y_axis_multiply), center.z - grid_interval/(lines_no/2)), V3(center.x + grid_interval/(lines_no/2), center.y + ((lines_no/2) * grid_interval * y_axis_multiply), center.z + grid_interval/(lines_no/2))});
                if (collision.hit) {
                    click_point_exists = true;
                    click_point_absolute = collision.point;
                    click_point = collision.point;

                    click_point.x = center.x;
                    click_point.z = center.z;

                    int vertical_sign = collision.point.y < 0 ? -1 : 1;

                    click_point.y = ((int) ((click_point.y + (grid_interval * vertical_sign/2))/ grid_interval)) * grid_interval;

                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        //center = click_point;
                        current_grid_y = click_point.y;
                        current_mode = 3;
                        y_axis_multiply = 1;
                    }

                } else {
                    click_point_exists = false;
                }
            } break;
            case 3:
            {
                //collision = GetCollisionRayGround(GetMouseRay(GetMousePosition(), camera), 0.0f);
                collision = GetRayCollisionQuad(GetMouseRay(GetMousePosition(), camera), V3(center.x - ((lines_no/2) * grid_interval), current_grid_y, center.z - ((lines_no/2) * grid_interval)), V3(center.x + ((lines_no/2) * grid_interval), current_grid_y, center.z - ((lines_no/2) * grid_interval)), V3(center.x + ((lines_no/2) * grid_interval), current_grid_y, center.z + ((lines_no/2) * grid_interval)), V3(center.x - ((lines_no/2) * grid_interval), current_grid_y, center.z + ((lines_no/2) * grid_interval)));
                if (collision.hit) {
                    click_point_exists = true;
                    click_point = collision.point;
                    click_point_absolute = collision.point;

                    float low_cutoff_x = (((int)(collision.point.x / grid_interval)) * grid_interval);
                    int sign_x = collision.point.x < 0 ? -1 : 1;
                    if ( (collision.point.x - low_cutoff_x) * sign_x < grid_interval / 2 ) {
                        click_point.x = low_cutoff_x;
                    } else {
                        click_point.x = low_cutoff_x + grid_interval * sign_x;
                    }

                    float low_cutoff_z = (((int)(collision.point.z / grid_interval)) * grid_interval);
                    int sign_z = collision.point.z < 0 ? -1 : 1;
                    if ( (collision.point.z - low_cutoff_z) * sign_z < grid_interval / 2 ) {
                        click_point.z = low_cutoff_z;
                    } else {
                        click_point.z = low_cutoff_z + grid_interval * sign_z;
                    }

                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        //current_mode = 1;
                        if (!check_for_point(all_points, current_point, click_point)) {
                            all_points[current_point++] = click_point;
                        }
                    }
                    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
                        current_mode = 1;
                    }
                } else {
                    click_point_exists = false;
                }
            } break;
        }

        if (current_mode == 2) {

        }

        if (current_mode == 3) {

        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                //DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
                //DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);


                //DrawGrid(10, 1.0f);

                // draw grid
                draw_cross(center, lines_no, grid_interval, BLACK);

                for (int i=0; i<current_point; i++) {
                    DrawSphere(all_points[i], 0.25, GREEN);
                    for (int j=0; j<selected_points_no; j++) {
                        if (i==selected_points[j]) {
                            DrawSphere(all_points[i], 0.30, SKYBLUE);
                        }
                    }
                }

                for (int i=0; i<faces_no; i++) {
                    DrawTriangle3D(all_points[faces[i].p1], all_points[faces[i].p2], all_points[faces[i].p3], BLUE);
                    DrawTriangle3D(all_points[faces[i].p3], all_points[faces[i].p2], all_points[faces[i].p1], BLUE);
                    //DrawTriangle3D(all_points[faces[i].p1], all_points[faces[i].p2], all_points[faces[i].p3], RED);
                }


                if (current_mode == 2) {
                    //draw_y_axis(center, 8, grid_interval);
                    //DrawBoundingBox((BoundingBox) {V3(center.x - grid_interval/(lines_no/2), center.y - ((lines_no/2) * grid_interval), center.z - grid_interval/(lines_no/2)), V3(center.x + grid_interval/(lines_no/2), center.y + ((lines_no/2) * grid_interval), center.z + grid_interval/(lines_no/2))}, BLUE);
                    int y_axis_height = lines_no;
                    if (click_point_exists) {
                        //DrawSphere(click_point_absolute, 0.25, BLUE);
                        DrawSphere(click_point, 0.25, RED);
                        draw_grid(click_point, lines_no, grid_interval, grid_color);
                        if (y_axis_multiply > 1) {
                            if (click_point.y < ((y_axis_height/2) * grid_interval) * (y_axis_multiply-1)) {
                                y_axis_multiply--;
                            }
                        }
                        if (click_point.y == ((y_axis_height/2) * grid_interval) * y_axis_multiply) {
                            y_axis_multiply++;
                        }
                    } else {
                        y_axis_multiply = 1;
                    }
                    draw_y_axis(center, y_axis_height * y_axis_multiply, grid_interval);
                }

                if (current_mode == 3) {
                    draw_grid(V3(center.x, current_grid_y, center.z), lines_no, grid_interval, grid_color);
                    if (click_point_exists) {
                        DrawSphere(click_point, 0.25, BLACK);
                        DrawSphere(click_point_absolute, 0.25, RED);
                    }
                }
            EndMode3D();

            if (current_mode == 1) {
                // let'em know that they can make a face if they selected 3 points
                if (selected_points_no == 1) {
                    DrawText("wanna move it?", 5, 0, 30, BLACK);
                }
                if (selected_points_no == 3) {
                    DrawText("bro you could make the fattest face rn [X]", 5, 0, 30, BLACK);
                }
            }

            if (current_mode == 3) {
                if (click_point_exists) {
                    sprintf(x_coord, "%.2f", click_point_absolute.x);
                    sprintf(z_coord, "%.2f", click_point_absolute.z);
                    DrawText(x_coord, 5, 0, 30, BLACK);
                    DrawText(z_coord, 5, 35, 30, BLACK);
                }
            }
            // grid_interval
            
            sprintf(grid_interval_val, "%.2f", grid_interval);
            DrawText(grid_interval_val, 700, 0, 50, BLACK);
            
            sprintf(lines_no_val, "%2d", lines_no);
            DrawText(lines_no_val, 700, 55, 50, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}