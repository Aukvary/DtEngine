#include "Ecs/RegisterHandler.h"
#include "raylib.h"
#define RAYLIB_NUKLEAR_INCLUDE_DEFAULT_FONT
#include "raylib-nuklear.h"

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800
struct nk_context *ctx;
struct nk_colorf bg;

void UpdateDrawFrame(void);

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "[raylib-nuklear] demo");
    SetTargetFPS(60);

    /* GUI */
    const int fontSize = 18;
    Font font = LoadFontFromNuklear(fontSize);
    ctx = InitNuklearEx(font, fontSize);
    bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;

    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }
    UnloadNuklear(ctx);
    CloseWindow();

    return 0;
}


void UpdateDrawFrame(void) {
    UpdateNuklear(ctx);

    if (nk_begin(ctx, "Demo", nk_rect(50, 50, 230, 250),
        NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
        NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
    {
        enum {EASY, HARD};
        static int op = EASY;
        static int property = 20;

        nk_layout_row_static(ctx, 30, 80, 1);

        u16 size;
        const DtComponentData** data = dt_component_get_all(&size);

        for (int i = 0; i < size; i++) {
            if (nk_button_label(ctx, data[i]->name)) {

            }
        }

        if (nk_button_label(ctx, "button")) {
            for (int i = 0; i < size; i++) {
                printf("%s\n", data[i]->name);
            }
        }

        nk_layout_row_dynamic(ctx, 30, 2);
        if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
        if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;
        nk_layout_row_dynamic(ctx, 22, 1);
        nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);

        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "background:", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 25, 1);
        if (nk_combo_begin_color(ctx, nk_rgb_cf(bg), nk_vec2(nk_widget_width(ctx),400))) {
            nk_layout_row_dynamic(ctx, 120, 1);
            bg = nk_color_picker(ctx, bg, NK_RGBA);
            nk_layout_row_dynamic(ctx, 25, 1);
            bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f,0.005f);
            bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f,0.005f);
            bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f,0.005f);
            bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f,0.005f);
            nk_combo_end(ctx);
        }
    }
    nk_end(ctx);

    BeginDrawing();
    {
        ClearBackground(ColorFromNuklearF(bg));
        DrawNuklear(ctx);
    }
    EndDrawing();
}