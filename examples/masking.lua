require "runlocal"
local Cairo = require "oocairo"

local IMG_WD, IMG_HT = 400, 300
local PI = 2 * math.asin(1)

-- This requires the GD library, and the Lua module for it.  We need it to
-- load the JPEG images.
do
    local ok = pcall(require, "gd")
    if not ok then
        print("The Lua 'gd' module is required for this example.")
        os.exit(1)
    end
end

function load_jpeg (filename)
    local img = assert(gd.createFromJpeg(filename),
                       "error loading JPEG: " .. filename)
    return Cairo.image_surface_create_from_png_string(img:pngStr())
end

function make_snow_gradient ()
    -- Fade from full opacity at the bottom of to full transparency at the top.
    local gradient = Cairo.pattern_create_linear(0, 0, 0, IMG_HT)
    gradient:add_color_stop_rgba(0, 0, 0, 0, 0)
    gradient:add_color_stop_rgba(0.3, 0, 0, 0, 0)
    gradient:add_color_stop_rgba(0.8, 0, 0, 0, 1)
    return gradient
end

function make_flower_gradient (size)
    -- Fade from full opacity in top left corner to more transparent in all
    -- directions.
    local gradient = Cairo.pattern_create_radial(size, size, 0,
                                                 size, size, size)
    gradient:add_color_stop_rgba(0.0, 0, 0, 0, 1)
    gradient:add_color_stop_rgba(0.8, 0, 0, 0, 1)
    gradient:add_color_stop_rgba(1.0, 0, 0, 0, 0)
    return gradient
end

function make_statue_mask (size)
    -- Set up a new image surface on which to drawn the mask shape.
    local surface = Cairo.image_surface_create("argb32", size, size)
    local cr = Cairo.context_create(surface)

    -- Draw an n-pointed star.
    local n = 7                     -- number of points
    local c = size / 2              -- center x/y position
    local r1, r2 = c - 2, c * .6    -- outer and inner radii
    local angstep = (2 * PI) / n
    for angmul = 1, n do
        local ang1 = angmul * angstep
        local ang2 = ang1 + angstep / 2
        cr:line_to(c + r1 * math.cos(ang1), c + r1 * math.sin(ang1))
        cr:line_to(c + r2 * math.cos(ang2), c + r2 * math.sin(ang2))
    end
    cr:close_path()

    cr:set_source_rgba(0, 0, 0, 1)
    cr:fill()

    return surface
end

local surface = Cairo.image_surface_create("rgb24", IMG_WD, IMG_HT)
local cr = Cairo.context_create(surface)

local bronzepic = load_jpeg("examples/images/bronze.jpg")
local flowerpic = load_jpeg("examples/images/flowers.jpg")
local snowpic   = load_jpeg("examples/images/snow.jpg")

-- Show the snowy scene at the bottom of the picture, faded out at the top,
-- using a linear gradient as the mask.
cr:set_source_surface(snowpic, 0, IMG_HT - select(2, snowpic:get_size()))
cr:mask(make_snow_gradient())

-- Show the flowers in a faded circle, with the image scaled to half size,
-- by maskign with a radial gradient.
cr:scale(0.5, 0.5)
cr:set_source_surface(flowerpic, 0, 0)
cr:identity_matrix()
cr:mask(make_flower_gradient(select(2, flowerpic:get_size()) / 4))

-- Show the bronze statue in a sharply defined cut-out shape, by drawing the
-- shape onto a blank transparent surface first, and then using that surface
-- as the mask.
cr:set_source_surface(bronzepic, 180, 30)
cr:mask_surface(make_statue_mask(200), 200, 0)

surface:write_to_png("masking.png")

-- vi:ts=4 sw=4 expandtab