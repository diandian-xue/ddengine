local ddgl = require "lddgl.core"
local gl = ddgl.opengl
local inspect = require "inspect"
local ddcl = require "lddcl.core"

local VAO, VBO, EBO
local program
local vert = [[
    #version 330 core
    layout (location = 0) in vec3 position;
    layout (location = 1) in vec4 color;

    out vec4 vcolor;
    //uniform mat4 transform;
    void main()
    {
        gl_Position = vec4(position, 1.0);
        gl_Position.z = 0;
        vcolor = color;
    }
]]
local frag = [[
    #version 330 core
    out vec4 FragColor;
    in vec4 vcolor;
    void main()
    {
        FragColor = vcolor;
    }
]]

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

local function init()
    VAO = gl.glGenVertexArrays(1)[1]
    VBO = gl.glGenBuffers(1)[1]
    EBO = gl.glGenBuffers(1)[1]

    local fsz = gl.sizeof(gl.GL_FLOAT)

    gl.glBindVertexArray(VAO)
    gl.glBindBuffer(gl.GL_ARRAY_BUFFER, VBO)

    gl.glVertexAttribPointer(0, 3, gl.GL_FLOAT, true, 3 * fsz + 4, 0)
    gl.glEnableVertexAttribArray(0)
    gl.glVertexAttribPointer(1, 4, gl.GL_UNSIGNED_BYTE, false, 3 * fsz + 4, 3 * fsz)
    gl.glEnableVertexAttribArray(1)

    gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, EBO)
    local indices = string.pack("=BBBBBB", 0, 1, 2, 3, 2, 1)
    gl.glBufferData(gl.GL_ELEMENT_ARRAY_BUFFER, gl.GL_STATIC_DRAW, indices)

    program = gl.glCreateProgram(vert, frag)
end

local function new()
    local M = {}

    local _childs   = {}
    local _parent   = nil
    local _position = { 0, 0, 0 }
    local _size     = { 0, 0 }
    local _scale    = { 1, 1 }
    local _rotation = { 0, 0, 0 }
    local _color    = { 255, 255, 255, 255 }

    local _matrix  = ddgl.new_matrix()
    local _dirty   = true
    local _visible = true

    local meta = {}
    meta.__gc = function()
        ddgl.free_matrix(_matrix)
    end
    setmetatable(M, meta)

    function M.setposition(position) _position = position _dirty = true end
    function M.setx(x) _position[1] = x _dirty = true end
    function M.sety(y) _position[2] = y _dirty = true end
    function M.setz(z) _position[3] = z _dirty = true end

    function M.setsize(size) _size = size end

    function M.setscale(scale) _scale[1] = scale _scale[2] = scale end
    function M.setscalex(x) _scale[1] = x end
    function M.setscaley(y) _scale[2] = y end

    function M.setrotation(angle) _rotation[3] = angle _dirty = true end
    function M.setrotationx(angle) _rotation[1] = angle _dirty = true end
    function M.setrotationy(angle) _rotation[2] = angle _dirty = true end

    function M.setcolor(color) _color = color end

    function M.update_matrix()
        ddgl.identity_matrix(_matrix)
        ddgl.translate_matrix(_matrix, _position)
        if _rotation[1] then ddgl.rotate_x_matrix(_matrix, _rotation[1]) end
        if _rotation[2] then ddgl.rotate_y_matrix(_matrix, _rotation[2]) end
        if _rotation[3] then ddgl.rotate_z_matrix(_matrix, _rotation[3]) end
    end

    function M.visit(pm, force)
        if not _visible then return end
        if _dirty or force then
            M.update_matrix()
        end

        for i, v in ipairs(_childs) do
            v.visit(_matrix)
        end
    end

    function M.render()
        local hw = _size[1] * _scale[1] * 0.5/ 960
        local hh = _size[2] * _scale[2] * 0.5 / 640
        local vertices = {
            -hw, hh, 0,     _color[1], _color[2], _color[3], _color[4],
            -hw, -hh, 0,    _color[1], _color[2], _color[3], _color[4],
            hw, hh, 0,      _color[1], _color[2], _color[3], _color[4],
            hw, -hh, 0,     _color[1], _color[2], _color[3], _color[4],
        }
        --print(inspect(vertices))
        local s = string.pack("=fffBBBBfffBBBBfffBBBBfffBBBB", table.unpack(vertices))
        gl.glBindVertexArray(VAO)
        gl.glBindBuffer(gl.GL_ARRAY_BUFFER, VBO)
        gl.glBufferData(gl.GL_ARRAY_BUFFER, gl.GL_STATIC_DRAW, s)
        gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, EBO)
        print(VAO, VBO, EBO)

        gl.glUseProgram(program)
        --gl.glUniformMatrix(program, "transform", _matrix)
        gl.glDrawElements(gl.GL_TRIANGLES, 3, gl.GL_UNSIGNED_BYTE, 0)
        print(gl.glGetError())
        ddcl.sleepms(10000)
    end


    return M
end


return {
    new = new,
    init = init,
}
