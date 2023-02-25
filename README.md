# CG_Final

## 技术细节

### 1. 延迟渲染

本次大作业摒弃了简单的前向渲染，采用基于帧缓冲的G-Buffer。这样，最终的输出就是由G-Buffers合成后的贴图，这样的性能就更高，因为在摄像头视线以外的片元不渲染。具体过程如下：

<img src="https://learnopengl.com/img/advanced-lighting/deferred_overview.png" title="" alt="Overview of the deferred shading technique in OpenGL" data-align="center">

先将所有的片元按照以往的方式渲染，但这次并不输出颜色FragColor，而是将纹理信息、位置信息、高光信息和法线信息存储到四张贴图。这个过程叫做G-Buffer。也就是我们把原有的渲染方式通过G-Buffer输出到四张贴图中。最后，通过后处理技术，再进行渲染。

这样的好处是我们将所有的信息都存到了贴图中，这样最后渲染的时候，不重要的信息就不会渲染了。优化了性能。这个技术广泛应用在各种游戏中。但坏处就是，在延迟渲染中Blend就失效了。为了渲染透明物体，我们应当适当的将延迟渲染和前向渲染结合在一起。

我们专门用来G-Buffer的着色器统一用gbuffers_xxx.vsh/fsh来命名，而后处理统一用compositex.vsh/fsh来命名，本大作业的shaders文件结构大致如下：

<img src="https://cdn.jsdelivr.net/gh/InverseDa/image@master/image/image-20230223145217764.png" title="" alt="image-20230223145217764" data-align="center">

为了实现这个技术，我们需要用到OpenGL中比较强大的帧缓冲（Frame Buffer）来实现G-Buffer，这个帧缓冲将输出四张贴图。

```cpp
class FrameBuffer {
public:
    GLuint fbo, rbo;
    // 四张贴图分别是位置贴图（其实这是最不需要的，因为可以用深度图重建坐标
    // 法线贴图
    // 漫反射贴图（A通道为高光强度）
    // 0: Position; 1: Normal; 2: Diffuse + specular
    GLuint Position, Normal, DiffuseSpecular, DepthTex;
    GLuint DrawBuffers[3];

    bool init(unsigned int WINDOW_WIDTH, unsigned int WINDOW_HEIGHT) {
        // create fbo
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        // create gbuffer texture
        glGenTextures(1, &Position);
        glBindTexture(GL_TEXTURE_2D, Position);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Position, 0);
        // normal color buffer
        glGenTextures(1, &Normal);
        glBindTexture(GL_TEXTURE_2D, Normal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, Normal, 0);
        // color + specular color buffer
        glGenTextures(1, &DiffuseSpecular);
        glBindTexture(GL_TEXTURE_2D, DiffuseSpecular);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, DiffuseSpecular, 0);

        // depth color buffer
        glGenTextures(1, &DepthTex);
        glBindTexture(GL_TEXTURE_2D, DepthTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
                     NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthTex, 0);

        DrawBuffers[0] = GL_COLOR_ATTACHMENT0;
        DrawBuffers[1] = GL_COLOR_ATTACHMENT1;
        DrawBuffers[2] = GL_COLOR_ATTACHMENT2;
        glDrawBuffers(3, DrawBuffers);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        return true;
    }
};
```

这样就创建了一个帧缓冲类，可以利用这个类创建一个G-Buffer。

```cpp
FrameBuffer* gbuffer;
// 窗口的长和宽，注意如果是APPLE的视网膜屏幕，需要将数值乘以2
gbuffer->init(SCR_WIDTH, SCR_HEIGHT);
```

因为我们采用的是指针开辟内存，所以用完需要回收内存空间。

### 2. 基于CPU计算和高度图的地形生成算法

高度图一是张黑白的贴图，其颜色属性代表的含义是越黑即对应的片元$y$轴越小，越白即对应的片元$y$轴越大。通过渲染一张高度图，就可以生成对应的地形。因为这个算法是在CPU计算出来的，所以叫做基于CPU计算和高度图的地形生成算法。这个算法的时间复杂度是$O(n^2)$，但由于地图不大，并且一旦生成完毕便不再执行，所以在一定的地图规模下，这个算法还是相对优的。<img title="" src="file:///Users/miaokeda/Library/Application%20Support/marktext/images/2023-02-23-15-17-33-image.png" alt="" data-align="center">

对于任何一张贴图，其数据结构都是二维数组。基于这个理论，我们可以得出高度图的大致结构：

<img src="https://learnopengl.com/img/guest/2021/tessellation/height_map/mesh_ij.png" title="" alt="Mesh Vertices" data-align="center">

左上角就是高度图的左上角，其他位置都是同理。

我们以高度图的中央作为坐标系的原点，左上角为-x和-z方向，右下角为x和z方向。高度图中每个元素的值就是海拔的值y。所以我们通过读取高度图的值，就能确定这个地形的所有信息。

```cpp
    int dwidth, dheight, dn;
    unsigned char* heightMap =stbi_load("textures/DefaultTerrain/Height Map.png", &width, &height, &nChannels, 0);

    if (!heightMap) {
        std::cout << "Error: Load HeightMap Failed! \n";
        exit(0);
    }
    float yScale = 256.0f / 256.0f, yShift = 16.0f;
    for (unsigned int i = 0; i < height; i++) {
        for (unsigned int j = 0; j < width; j++) {
            unsigned char* texel = heightMap + (i * width + j) * nChannels;
            unsigned char y = texel[0];
            float xx = -height / 2.0f + i, yy = (int)y * yScale - yShift,
                  zz = -width / 2.0f + j;
            terrainVertices.push_back(xx);
            terrainVertices.push_back(yy);
            terrainVertices.push_back(zz);
            // Texcoords UV Coordinate
            terrainVertices.push_back((j)*1.0f / (float)(width - 1));
            terrainVertices.push_back((i)*1.0f / (float)(height - 1));
        }
    }

    stbi_image_free(heightMap);

    //  indices calculation
    for (unsigned int i = 0; i < width - 1; i++) {
        for (unsigned int j = 0; j < width; j++) {
            for (unsigned int k = 0; k < 2; k++) {
                terrainIndices.push_back((i + k) * width + j);
            }
        }
    }
```

为了优化性能，我们需要做切片。也就是利用Element Buffer来定义面片切片。这里给出一种方法：

<img src="https://learnopengl.com/img/guest/2021/tessellation/height_map/mesh_strips.png" title="" alt="Mesh Triangle Strips" data-align="center">

从正序的方向遍历（即先i后j），组成如上图所示的三角面片。为了达到这样的效果，可以按照这样的顺序构造面片：

<img src="https://learnopengl.com/img/guest/2021/tessellation/height_map/mesh_strip_generation.png" title="" alt="Mesh Triangle Strip Numbering" data-align="center">

我们可以按照012345的顺序存储切片序列，这样012是一个三角形，345是一个三角形...推广至任意位置的：

<img src="https://learnopengl.com/img/guest/2021/tessellation/height_map/mesh_strip_generation_generic.png" title="" alt="Mesh Triangle Strip Generic Numbering" data-align="center">

注意到我们没有渲染123，234的三角形，这是因为这样可以优化算法，避免多次计算。因为012和345可以合成一个矩形，在误差允许范围内这是可行的。

绘制方式如下：

```cpp
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, terrainTextureID);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, terrainSpecular);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, terrainNormal);
        terrainShader->use();
        terrainShader->set4Matrix("model", model);
        terrainShader->set4Matrix("view", view);
        terrainShader->set4Matrix("projection", projection);
        terrainShader->setInt("tex", 0);
        terrainShader->setInt("specular", 1);
        terrainShader->setInt("normal", 2);
        // draw mesh
        glBindVertexArray(terrainVAO);
        // render the mesh triangle strip by triangle strip - each row at a time
        for (unsigned int strip = 0; strip < height - 1; ++strip) {
            glDrawElements(GL_TRIANGLE_STRIP, // primitive type
                           (width * 2),       // number of indices to render
                           GL_UNSIGNED_INT,   // index data type
                           (void*)(sizeof(unsigned int) * (width * 2) *
                                   strip)); // offset to starting index
        }
```

### 3. 阴影映射和PCF软阴影

定义一个专门渲染阴影的帧缓冲，然后先以光源为视角渲染出一张灰度的深度贴图。给阴影上色的时候先判断当前深度和深度图的深度大小，如果小的话说明需要上阴影，于是涂黑。具体思路如图所示：
