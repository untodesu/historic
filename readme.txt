This is in really early stage of development but
so far I've achieved some progress in ECS-based
design and it should be at least interesting to
look at.

For now there's only a quad and a FPS noclip camera
with the following controls:
    WASD - moving sideways
    SPACE - moving upwards
    LEFT SHIFT - moving downwards
    MOUSE - looking around (still FPS-dependent...)

To-do list so far:
    [x] At least basic client-server design. Even
        without a working server yet.
    [x] Thread pool mesh generation.
    [x] Greedy meshing for solid voxels.
    [x] Basic voxel removal by client.
    [ ] Basic diffuse lighting because things look messy.
    [ ] At least some of __working__ client-server code.

Issues known so far and things that fix them:
    1.  Mesh generation in Win32/Debug is INCREDIBLY
        slow. Having neighbouring chunks loaded within
        the ChunkMesherData speeds it up a little but
        with no good results. Win32/Release is much much
        faster. This is really strange considering hash
        table search should have complexity of O(1) or O(n).
        But this shit is slow even if there's two chunks!
    2.  No serverside. This will be fixed when I figure out
        a nice way to send chunk data without hardcore shitposting
        like converting 16-bit integers from native and back. Also
        I probably will borrow some OpenBuilder code for that.
    3.  Performance. There we go. It works better now though.

Plans for rendering techniques:
    1.  Voxelius would use deferred shading because it seems
        like a better way for rendering lots of vertices per-frame.
        Though there would be a bunch of GBuffers for different types
        of models (like OptiFine does that): solid (including flora) voxels,
        transparent (texture-wise) voxels, liquid & gas voxels, entities.
        Things would be combined in the composite shader.
    2.  I probably will do some research and try to make colored shadows.
        What did you expect from a OpenGL geek?
    3.  Something else? Maybe some atmospheric scattering because I find it
        rather awesome to have fancy sky in a blocky game.

Plans for gameplay:
    1.  I want to resemble Minecraft in most ways because I just want
        to have a free alternative to it without needing to have a
        whole new Micro$oft account. So no fancy non-vanilla things.
    2.  Unlike MC, chunks will remain cubic and the gameplay mechanics
        will be built around that fact.
    3.  Different dimensions - maybe.
    4.  World generation - flying islands with biomes and shit.
    5.  Water - like in Terraria if I will be able to implement such water in 3D.
    6.  Mods - yes, probably. Filesystem is already capable of mounting things.
