# An implementation of [_A narrow range filter for screen space fluid dynamics_](https://dl.acm.org/doi/10.1145/3203201)

## Information

Loads a fluid simulation through `.abc` (alembic) files.

## Build Information

### External Libraries

- [cyCodeBase](https://github.com/cemyuksel/cyCodeBase)
- [glew-2.1.0](https://glew.sourceforge.net/)
- [glfw-3.4.0](https://github.com/glfw/glfw)
- [opengl](https://www.opengl.org/)
- [lodepng](https://lodev.org/lodepng/)
- [alembic](https://github.com/alembic/alembic)

To add cyCodeBase, glew-2.1.0, glfw, and lodepng, download them and add to a `libs` folder in the project directory.
Alembic, Imath and OpenGL should be in a location where CMake is able to find the project.
