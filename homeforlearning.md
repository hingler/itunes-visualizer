# shit i do

- For some reason there is an inconsistent issue with read/write to/from buffers
- it's almost certainly some type of desynchronization but the reason behind it is unclear, as the way the calls are set up should avoid it
- however, it appears to be relatively minor (at least for now)

- so i am going to ignore it >:)

# GLFW

## Key callbacks

The PollEvents function runs into the issue that if a user presses and releases a key in the time taken prior to polling events, the input will be ignored. GLFW suggests two solutions to this:

- glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
- glfwSetKeyCallback(window, callback);

Seemingly you would want to use the poll approach for keys which are held down, and the callback approach for keys which are just pressed.

Cool shit: https://www.glfw.org/documentation.html
---

# VAO

Kronos wiki specifies that a VAO records everything about the vertex data which we intend to pass to our vert shader. This includes calls to glEnable(/Disable)VertexAttribArray and glVertexAttribPointer.
Also keeps track of the EBO.

This is only available in #version 300 core and above.

