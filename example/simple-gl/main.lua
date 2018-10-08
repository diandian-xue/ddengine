local ENGINE_ROOT = os.getenv("DDENGINE_ROOT")
local path = {
    "./lualib/?.lua",
    "./example/simple-gl/?.lua",

    ENGINE_ROOT .. "/lualib/?.lua",
    ENGINE_ROOT .. "/example/simple-gl/?.lua",

}
package.path = table.concat(path, ";")

local ddcl = require "lddcl.core"
local ddgl = require "lddgl.core"
local cjson = require "cjson"
local inspect = require "inspect"
local gl = ddgl.opengl
local ltexture = require "ltexture.core"

local conf = {
    worker = 1,
    socket = 1,
    timer_ms = 1,
}
ddcl.init(conf)
ddgl.init()

local function p_vec(vec)
    print(inspect(ddgl.table_vec(vec)))
end

local function p_matrix(m)
    local t = ddgl.table_matrix(m)
    local s = {"{"}
    for i = 1, 4 do
        local line = {}
        for vi = 1, 4 do
            local index = (vi - 1) * 4 + i
            local num = t[index]
            table.insert(line, num)
        end
        table.insert(s, table.concat(line, ", "))
    end
    table.insert(s, "}\n")
    print(table.concat(s, "\n"))
end

local function loadpng()
    local face = ltexture.new_font_face("C:\\Windows\\fonts\\simhei.ttf", 32)
    local http = require "http"
    http.init()
    local respone = http.get("119.23.216.216", 30001, "/", {})

    local txdef = {
        maxw = 1000,
        maxh = 500,
    }
    local data, width, height = ltexture.data_with_string(face, respone.body, {})

    --local data, width, height = ltexture.decode_png_file("logo.png")
    print(data, width, height)

    local tx = gl.glGenBuffers(1)
    tx = tx[1]
    print(tx)
    gl.glBindTexture(gl.GL_TEXTURE_2D, tx)
    --local sw = { gl.GL_RED, gl.GL_GREEN, gl.GL_BLUE, gl.GL_ALPHA }
    local sw = { gl.GL_ONE, gl.GL_ONE, gl.GL_ONE, gl.GL_ALPHA }
    gl.glTexParameteriv(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_SWIZZLE_RGBA, sw)
    gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MIN_FILTER, gl.GL_LINEAR);
    gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MAG_FILTER, gl.GL_LINEAR);
    gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_S, gl.GL_CLAMP_TO_EDGE);
    gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_T, gl.GL_CLAMP_TO_EDGE);
    gl.glTexImage2D(gl.GL_TEXTURE_2D, 0, gl.GL_RGBA8, width, height, gl.GL_RGBA, gl.GL_UNSIGNED_BYTE, data);

    ltexture.free(data)
    return tx;
end


ddcl.start_non_worker(function(self, from, session, ptype, cmd, data, sz)

    ddcl.callback(function(self, from, session, ptype, cmd, data, sz)
    end)

    local win_width = 960
    local win_height = 640
    local w = ddgl.new_window(win_width, win_height)
    ddgl.show_window(w)
    ddgl.init_modules()
    print("error:", gl.glGetError())

    local texture = loadpng()
    ddgl.render_image(texture)
    --ddcl.co_sleep(10000)

    local last_t = ddcl.now()
    local tcount = 0
    local function pollev()
        ddcl.timeout(32, pollev)


        ddgl.render_window(w)
        gl.glClear()

        ddgl.render_image(texture)

        --sp.visit(nil, false)
        --sp.render()

        ddgl.poll_window_event()
        tcount = tcount + 1

        if (ddcl.now() - last_t) > 1000 then
            last_t = ddcl.now()
            print(tcount, gl.glGetError())
            tcount = 0
        end
    end
    pollev()

    local function render()
        ddcl.timeout(18, render)

        ddgl.swap_buffer(w)
    end
    render()
end)
