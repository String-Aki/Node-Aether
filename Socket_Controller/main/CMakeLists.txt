idf_component_register(
    SRCS
        "main.c"
        "display_manager.c"
        "fonts/ui_font_orbitron_12.c"
        "fonts/ui_font_orbitron_16.c"
        "fonts/ui_font_orbitron_20.c"
        "fonts/ui_font_orbitron_24.c"
        "fonts/ui_font_share_tech_mono_12.c"
        "fonts/ui_font_share_tech_mono_16.c"
        "fonts/ui_font_share_tech_mono_20.c"
        "fonts/ui_font_share_tech_mono_24.c"
        "fonts/ui_font_silkscreen_12.c"
        "fonts/ui_font_silkscreen_16.c"
        "fonts/ui_font_silkscreen_20.c"
        "fonts/ui_font_silkscreen_24.c"
        "fonts/ui_font_vt323_12.c"
        "fonts/ui_font_vt323_16.c"
        "fonts/ui_font_vt323_20.c"
        "fonts/ui_font_vt323_24.c"
    INCLUDE_DIRS "." "fonts"
    REQUIRES esp_lcd lvgl
)