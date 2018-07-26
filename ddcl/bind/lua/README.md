
This is Lua 5.3.3, released on 30 May 2016.

For installation instructions, license details, and
further information about Lua, see doc/readme.html.

## 编译
	clone https://github.com/diandian-xue/lua.git
	cd lua

### Windows
	使用cmake-gui创建visual studio项目

### Unix
	cmake ./ -DLINUX=1

### 编译动态库
	cmake时添加 -DBUILD_SHARED_LIBS=1