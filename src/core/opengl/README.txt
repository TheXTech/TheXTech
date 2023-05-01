To all who are interested in TheXTech's OpenGL renderer:


Principles:
- The renderer is designed to support the most common advanced graphical effects very efficiently, but not to support the kind of freedom provided by the SMBX2 renderer.
- In particular, user-specified render targets are an explicit non-goal of the renderer.
- The renderer aims to be user-friendly and to minimize the number of surprises that content creators might face.
- A primary aim of the renderer is to ensure that the best (most maintainable and interoperable) way to do something is the easiest.
- The renderer attempts to abstract over the wide variety of hardware by exposing a limited number of quality profiles.


Renderer design:
- The renderer is based on a deferred drawing model for efficiency and to enable multipass render effects.
- The renderer batches draw calls by their "context", which is formed by a GL program object, a texture, and possible the program's uniform state.
- External calls into the renderer fill draw queues; calls which draw fully opaque objects are entered into their context's unordered draw queue, and calls which draw translucent objects or have complex programs are entered into their context's ordered draw queue.
- During execution of the unordered draw queue, the depth test is enabled and the depth buffer is updated. The ordered draw queue is depth-sorted, and it is executed with the depth test enabled but without changing the depth buffer.
- There are a number of complex functions a GL program might attempt to perform, and these are supported in the function to execute the ordered draw queue:
  - Reading from the framebuffer: if the draw is a single rect, that rect is copied from the framebuffer to a fb read texture. Otherwise, the entire framebuffer is copied to the fb read texture. This texture is made available to the fragment shader.
  - Reading from the previous pass: the ordered draw queue is drawn multiple times, to two auxiliary textures and the primary render texture as appropriate. During each pass, the target is initialized to the opaque state, and these programs are provided with the render texture from the previous pass.
  - Reading from the depth buffer: following the unordered draws, the depth buffer is copied to a depth texture which is later made available to these programs.
  - Reading from the lighting buffer: if any shaders perform this read, the lighting state is calculated following the unordered draws and the result is made available to these programs.
- Whenever the overall draw state (viewport or render target) changes, and at the end of the frame, the draw queue is flushed and all draw calls are executed.
- Although drawing is deferred, the renderer manages GL programs' uniform state to maintain the same semantics as in immediate drawing.
- In nearly all cases, a standard vertex representation is used, and only the fragment shader may be modified.
- Particle systems are a special case, and their state is stored on GPU with only minimal per-frame updates.


Quality profiles and OpenGL version requirements:
- The renderer flexibly scales to versions of OpenGL from 1.1 to 4.3+, and exposes three quality profiles to the external game: low, medium, and high. Content may choose to require medium or high quality, with all content strongly encouraged to support medium quality.
  - Low quality rendering (GL 1.1+, GL ES 1.1+) does not support custom user shaders, and does not require render-to-texture.
  - Medium quality rendering (GL 3.3+, GL ES 2.0+) supports GLSL ES 100 (GL ES 2.0) shaders, which may read from the framebuffer or from the previous pass. Multipass rendering is limited to 2 passes. Particle systems are supported with a particle cap of 256 (maybe 512?) per system. Lighting is not currently supported, but may be in the future -- if so it will not support shadows.
  - High quality rendering (GL 4.3+, GL ES 3.0+) supports GLSL ES 300 (GL ES 3.0) shaders, which may additionally read from the depth buffer. Multipass rendering performs 4 passes. Particle systems are supported with a cap of 4196 particles per system. Lighting supports full dynamic shadowing. Users may write GLSL ES 100 or GLSL ES 300 shaders targeting high quality, with the expectation that they may use more resources than medium quality ones.
- Some of the key differences between GL versions include the following:
  - Desktop versions of GL (and GL ES 1.1) support native logic op rendering, which is used to support the bitmask effects of SMBX 1.3. They are fully emulated on GL ES 2.0+ using shaders.
  - GL ES 2.0+ and desktop GL 3.0+ support framebuffer objects. These are required for user shader support and are used extensively to support the following features:
    - Render-to-texture
    - Reading copies of portions of the framebuffer from shaders
    - Accessing previous passes during multipass rendering
  - Desktop GL 3.3+ and GL ES 2.0+ support GLSL ES 100 shaders, which are used extensively for medium quality rendering.
  - Desktop GL 3.0+ and GL ES 3.0+ support depth textures. These are used for shaders to read from the depth buffer and required for shadowing. In GL ES 2.0, a depth renderbuffer is used to support the main framebuffer object.
  - Desktop GL 3.1+ and GL ES 3.0+ support uniform buffer objects, which are used to pass data to the lighting program.
  - Desktop GL 4.3+ and GL ES 3.0+ support GLSL ES 300 shaders, which are used for advanced rendering.
  - WebGL and GL Core profiles do not support client-side arrays; thus all draw data is copied to vertex buffer objects prior to rendering. This reduces performance.
  - GL Core profiles require the use of a vertex attribute array. One is created and activated during initialization and left untouched for the rest of the program.


GL program objects:
-


Particle systems:
-


Lighting system:
-


Shader API (unimplemented):
-
