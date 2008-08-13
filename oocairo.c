#include <cairo.h>
#include <lua.h>
#include <lauxlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int luaopen_oocairo (lua_State *L);

#define MT_NAME_CONTEXT ("6404c570-6711-11dd-b66f-00e081225ce5")
#define MT_NAME_MATRIX  ("6e2f4c64-6711-11dd-acfc-00e081225ce5")
#define MT_NAME_PATH    ("6d83bf34-6711-11dd-b4c2-00e081225ce5")
#define MT_NAME_PATTERN ("6dd49a26-6711-11dd-88fd-00e081225ce5")
#define MT_NAME_SURFACE ("6d31a064-6711-11dd-bdd8-00e081225ce5")

static const char * const format_option_names[] = {
    "argb32", "rgb24", "a8", "a1", 0
};
static const cairo_format_t format_option_values[] = {
    CAIRO_FORMAT_ARGB32, CAIRO_FORMAT_RGB24, CAIRO_FORMAT_A8, CAIRO_FORMAT_A1
};

static const char * const antialias_names[] = {
    "default", "none", "gray", "subpixel", 0
};
static const cairo_antialias_t antialias_values[] = {
    CAIRO_ANTIALIAS_DEFAULT, CAIRO_ANTIALIAS_NONE, CAIRO_ANTIALIAS_GRAY,
    CAIRO_ANTIALIAS_SUBPIXEL
};

static const char * const linecap_names[] = {
    "butt", "round", "square", 0
};
static const cairo_line_cap_t linecap_values[] = {
    CAIRO_LINE_CAP_BUTT, CAIRO_LINE_CAP_ROUND, CAIRO_LINE_CAP_SQUARE
};

static const char * const linejoin_names[] = {
    "miter", "round", "bevel", 0
};
static const cairo_line_cap_t linejoin_values[] = {
    CAIRO_LINE_JOIN_MITER, CAIRO_LINE_JOIN_ROUND, CAIRO_LINE_JOIN_BEVEL
};

static const char * const fillrule_names[] = {
    "winding", "even-odd", 0
};
static const cairo_fill_rule_t fillrule_values[] = {
    CAIRO_FILL_RULE_WINDING, CAIRO_FILL_RULE_EVEN_ODD
};

static const char * const operator_names[] = {
    "clear",
    "source", "over", "in", "out", "atop",
    "dest", "dest-over", "dest-in", "dest-out", "dest-atop",
    "xor", "add", "saturate",
    0
};
static const cairo_operator_t operator_values[] = {
    CAIRO_OPERATOR_CLEAR,
    CAIRO_OPERATOR_SOURCE, CAIRO_OPERATOR_OVER, CAIRO_OPERATOR_IN,
    CAIRO_OPERATOR_OUT, CAIRO_OPERATOR_ATOP,
    CAIRO_OPERATOR_DEST, CAIRO_OPERATOR_DEST_OVER, CAIRO_OPERATOR_DEST_IN,
    CAIRO_OPERATOR_DEST_OUT, CAIRO_OPERATOR_DEST_ATOP,
    CAIRO_OPERATOR_XOR, CAIRO_OPERATOR_ADD, CAIRO_OPERATOR_SATURATE
};

static const char * const content_names[] = {
    "color", "alpha", "color-alpha", 0
};
static const cairo_content_t content_values[] = {
    CAIRO_CONTENT_COLOR, CAIRO_CONTENT_ALPHA, CAIRO_CONTENT_COLOR_ALPHA
};

static const char * const extend_names[] = {
    "none", "repeat", "reflect", "pad", 0
};
static const cairo_extend_t extend_values[] = {
    CAIRO_EXTEND_NONE, CAIRO_EXTEND_REPEAT, CAIRO_EXTEND_REFLECT,
    CAIRO_EXTEND_PAD
};

static const char * const filter_names[] = {
    "fast", "good", "best", "nearest", "bilinear", "gaussian", 0
};
static const cairo_filter_t filter_values[] = {
    CAIRO_FILTER_FAST, CAIRO_FILTER_GOOD, CAIRO_FILTER_BEST,
    CAIRO_FILTER_NEAREST, CAIRO_FILTER_BILINEAR, CAIRO_FILTER_GAUSSIAN
};

static void
to_lua_matrix (lua_State *L, cairo_matrix_t *mat, int pos) {
    double *matnums;
    int i;
    matnums = (double *) mat;
    for (i = 0; i < 6; ++i) {
        lua_pushnumber(L, matnums[i]);
        lua_rawseti(L, pos, i + 1);
    }
}

static void
create_lua_matrix (lua_State *L, cairo_matrix_t *mat) {
    lua_createtable(L, 6, 0);
    to_lua_matrix(L, mat, lua_gettop(L));
    luaL_getmetatable(L, MT_NAME_MATRIX);
    lua_setmetatable(L, -2);
}

static void
from_lua_matrix (lua_State *L, cairo_matrix_t *mat, int pos) {
    double *matnums;
    int i;
    luaL_checktype(L, pos, LUA_TTABLE);
    matnums = (double *) mat;
    for (i = 0; i < 6; ++i) {
        lua_rawgeti(L, pos, i + 1);
        if (!lua_isnumber(L, -1))
            luaL_error(L, "value %d in matrix isn't a number", i + 1);
        matnums[i] = lua_tonumber(L, -1);
        lua_pop(L, 1);
    }
}

#include "obj_context.c"
#include "obj_matrix.c"
#include "obj_path.c"
#include "obj_pattern.c"
#include "obj_surface.c"

static const luaL_Reg
constructor_funcs[] = {
    { "context_create", context_create },
    { "image_surface_create", image_surface_create },
    { "image_surface_create_from_png", image_surface_create_from_png },
    { "matrix_create", cairmat_create },
    { "pattern_create_for_surface", pattern_create_for_surface },
    { "pattern_create_linear", pattern_create_linear },
    { "pattern_create_radial", pattern_create_radial },
    { "pattern_create_rgb", pattern_create_rgb },
    { "pattern_create_rgba", pattern_create_rgba },
    { "surface_create_similar", surface_create_similar },
    { 0, 0 }
};

static void
add_funcs_to_table (lua_State *L, const luaL_Reg *funcs) {
    const luaL_Reg *l;
    for (l = funcs; l->name; ++l) {
        lua_pushstring(L, l->name);
        lua_pushcfunction(L, l->func);
        lua_rawset(L, -3);
    }
}

static void
create_object_metatable (lua_State *L, const char *mt_name,
                         const char *debug_name, const luaL_Reg *methods)
{
    if (luaL_newmetatable(L, mt_name)) {
        lua_pushliteral(L, "_NAME");
        lua_pushstring(L, debug_name);
        lua_rawset(L, -3);
        add_funcs_to_table(L, methods);
        lua_pushliteral(L, "__index");
        lua_pushvalue(L, -2);
        lua_rawset(L, -3);
    }
    lua_pop(L, 1);
}

int
luaopen_oocairo (lua_State *L) {
#ifdef VALGRIND_LUA_MODULE_HACK
    /* Hack to allow Valgrind to access debugging info for the module. */
    luaL_getmetatable(L, "_LOADLIB");
    lua_pushnil(L);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
#endif

    /* Create the table to return from 'require' */
    lua_newtable(L);
    lua_pushliteral(L, "_NAME");
    lua_pushliteral(L, "cairo");
    lua_rawset(L, -3);
    lua_pushliteral(L, "_VERSION");
    lua_pushliteral(L, VERSION);
    lua_rawset(L, -3);
    lua_pushliteral(L, "_CAIRO_VERSION");
    lua_pushstring(L, cairo_version_string());
    lua_rawset(L, -3);
    add_funcs_to_table(L, constructor_funcs);

    /* Create the metatables for objects of different types. */
    create_object_metatable(L, MT_NAME_CONTEXT, "cairo context object",
                            context_methods);
    create_object_metatable(L, MT_NAME_MATRIX, "cairo matrix object",
                            cairmat_methods);
    create_object_metatable(L, MT_NAME_PATH, "cairo path object",
                            path_methods);
    create_object_metatable(L, MT_NAME_PATTERN, "cairo pattern object",
                            pattern_methods);
    create_object_metatable(L, MT_NAME_SURFACE, "cairo surface object",
                            surface_methods);

    return 1;
}

/* vi:set ts=4 sw=4 expandtab: */