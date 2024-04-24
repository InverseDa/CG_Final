# CG_Final : 2020级计算机图形学期末大作业

## 分支介绍

- main：此分支为重构后的版本
- old：原先的版本，不推荐使用，留档用

## 构建

### Windows

使用cmake构建：

```bash
cmake -B .
```

构建完毕后打开sln文件即可

### Arch Linux

对于Arch Linux用户，需要用pacman安装glfw和assimp

```bash
sudo pacman -S glfw assimp
```

安装后在项目根目录下使用

```bash
cmake -B .; make
```

接下来将可执行文件设为可启动

```bash
sudo chmod +x CG_Final
```

最后执行即可。

### Fedora

对于Fedora用户，需要用dnf安装以下四个包
```bash
sudo dnf -y install glfw glfw-devel assimp assimp-devel
```

安装后在项目根目录下使用

```bash
cmake -B .; make
```

接下来将可执行文件设为可启动

```bash
sudo chmod +x CG_Final
```

## MacOS

安装后在项目根目录下使用

```bash
cmake -B .; make
```

接下来将可执行文件设为可启动

```bash
sudo chmod +x CG_Final
```

最后执行即可。

## 技术细节

### 1. 延迟渲染管线

本次大作业摒弃了简单的前向渲染，采用基于帧缓冲的G-Buffer。这样，最终的输出就是由G-Buffers合成后的贴图，这样的性能就更高，因为在摄像头视线以外的片元不渲染。具体过程如下：

![](https://learnopengl.com/img/advanced-lighting/deferred_overview.png)

先将所有的片元按照以往的方式渲染，但这次并不输出颜色FragColor，而是将纹理信息、位置信息、高光信息和法线信息存储到四张贴图。这个过程叫做G-Buffer。也就是我们把原有的渲染方式通过G-Buffer输出到四张贴图中。最后，通过后处理技术，再进行渲染。

这样的好处是我们将所有的信息都存到了贴图中，这样最后渲染的时候，不重要的信息就不会渲染了。优化了性能。这个技术广泛应用在各种游戏中。但坏处就是，在延迟渲染中Blend就失效了。为了渲染透明物体，我们应当适当的将延迟渲染和前向渲染结合在一起。

我们专门用来G-Buffer的着色器统一用gbuffers_xxx.vsh/fsh来命名，而后处理统一用compositex.vsh/fsh来命名，本大作业的shaders文件结构大致如下：

```
./shaders:
├── composite1.fsh
├── composite1.vsh
├── composite2.fsh
├── composite2.vsh
├── debug.fsh
├── debug.vsh
├── final.fsh
├── final.vsh
├── gbuffers_model.fsh
├── gbuffers_model.vsh
├── gbuffers_robot.fsh
├── gbuffers_robot.vsh
├── gbuffers_sun.fsh
├── gbuffers_sun.vsh
├── gbuffers_terrain.fsh
├── gbuffers_terrain.vsh
├── gbuffers_water.fsh
├── gbuffers_water_reflection.fsh
├── gbuffers_water_reflection.vsh
├── gbuffers_water.vsh
├── shadow.fsh
├── shadow.vsh
├── sky.fsh
└── sky.vsh
```

为了实现这个技术，我们需要用到OpenGL中帧缓冲（FrameBuffer）来实现G-Buffer，这个帧缓冲将输出四张贴图。

```cpp
class FrameBuffer {
  protected:
    int m_width;
    int m_height;
    GLuint m_fbo;
    GLuint m_rbo;
    std::unordered_map<std::string, GLuint> m_textures;
    std::vector<GLenum> m_drawBuffers;

    static std::shared_ptr<FrameBuffer> CreateFrameBuffer(int width, int height, std::vector<std::pair<std::string, TextureInfo>> attachments);

  public:
    struct Builder {
        int width;
        int height;
        std::vector<std::pair<std::string, TextureInfo>> attachments;

        Builder(int width, int height) : width(width), height(height) {}
        Builder& SetAttachment(const std::string& name,
                               GLenum internalFormat,
                               GLenum format,
                               GLenum type,
                               GLenum attachment,
                               bool flag,
                               std::vector<std::pair<GLenum, GLfloat>> params = {
                                   {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
                                   {GL_TEXTURE_MAG_FILTER, GL_LINEAR},
                               }) {
            attachments.push_back({name, TextureInfo{internalFormat, format, type, attachment, flag, params}});
            return *this;
        }
        std::shared_ptr<FrameBuffer> Build() {
            return FrameBuffer::CreateFrameBuffer(width, height, attachments);
        }
    };

    FrameBuffer(int width, int height, std::vector<std::pair<std::string, TextureInfo>> attachments);
    ~FrameBuffer();
    void Bind();
    void UnBind();
    glm::vec2 GetSize() const { return {m_width, m_height}; };
    GLuint GetTexture(std::string name) const { return m_textures.at(name); };
};
```

这样就创建了一个帧缓冲类，可以利用这个类创建一个G-Buffer。

```cpp
auto gbuffer = FrameBuffer::Builder(width, height)
                        .SetAttachment("position", GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT0)
                        .SetAttachment("normal", GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT1)
                        .SetAttachment("albedoSpec", GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT2)
                        .SetAttachment("depth", GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_ATTACHMENT, false, GetDepthTextureParams())
                        .Build();
```

### 2. 基于CPU计算和高度图的地形生成算法

高度图一是张黑白的贴图，其颜色属性代表的含义是越黑即对应的片元y轴越小，越白即对应的片元y轴越大。通过渲染一张高度图，就可以生成对应的地形。因为这个算法是在CPU计算出来的，所以叫做基于CPU计算和高度图的地形生成算法。这个算法的时间复杂度是$O(n^2)$，但由于地图不大，并且一旦生成完毕便不再执行，所以在一定的地图规模下，这个算法还是相对优的。

![](https://cdn.jsdelivr.net/gh/InverseDa/image@master/image/Untitled.png)

对于任何一张贴图，其数据结构都是二维数组。基于这个理论，我们可以得出高度图的大致结构：

![](https://learnopengl.com/img/guest/2021/tessellation/height_map/mesh_ij.png)

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

![](https://learnopengl.com/img/guest/2021/tessellation/height_map/mesh_strips.png)

从正序的方向遍历（即先i后j），组成如上图所示的三角面片。为了达到这样的效果，可以按照这样的顺序构造面片：

![](https://learnopengl.com/img/guest/2021/tessellation/height_map/mesh_strip_generation.png)

我们可以按照012345的顺序存储切片序列，这样012是一个三角形，345是一个三角形...推广至任意位置的：

![](https://learnopengl.com/img/guest/2021/tessellation/height_map/mesh_strip_generation_generic.png)

注意到我们没有渲染123，234的三角形，这是因为这样可以优化算法，避免多次计算。因为012和345可以合成一个矩形，在误差允许范围内这是可行的。

绘制方式如下：

```cpp
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

![](https://cdn.jsdelivr.net/gh/InverseDa/image@master/image/Untitled%201.png)

注意到，我们之前还顺便传入了地形中每个顶点的UV坐标：

```glsl
...
// Texcoords UV Coordinate
terrainVertices.push_back((j)*1.0f / (float)(width - 1));
terrainVertices.push_back((i)*1.0f / (float)(height - 1));
...
```

因此，可以通过UV来进行地形的颜色绘制，着色器如下：

```glsl
// ======== VSH ========
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 texCoord;
in vec3 color;

out VS_OUT{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
}vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

uniform sampler2D normal;

void main(){
    vec3 norm = texture(normal, texCoord).rgb;
    vec3 position = vec3(aPos.x, aPos.y, aPos.z);
    vs_out.FragPos = vec3(model * vec4(position, 1.0f));
    vs_out.Normal = norm;
    vs_out.TexCoords = texCoord;
    gl_Position = projection * view * model * vec4(position, 1.0);
}

// ======== FSH ========
#version 330 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gDiffuseSpecular;
layout (location = 3) out vec4 gFeatureTex;

const int bottom = 250;
const int top = 300;
const int width = 1000;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D shadowMap;
uniform sampler2D noisetex;
uniform sampler2D watertex;
uniform sampler2D tex;
uniform sampler2D specular;
uniform sampler2D normal;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 lightDirection;

uniform mat4 model;

void main() {
    gPosition = fs_in.FragPos;
    gNormal = fs_in.Normal;
    gDiffuseSpecular.rgb = texture(tex, fs_in.TexCoords).rgb;
    gDiffuseSpecular.a = texture(specular, fs_in.TexCoords).r;
}
```

![](https://cdn.jsdelivr.net/gh/InverseDa/image@master/image/Untitled%202.png)

### 3. 阴影映射和PCF软阴影

定义一个专门渲染阴影的帧缓冲，然后先以光源为视角渲染出一张灰度的深度贴图。给阴影上色的时候先判断当前深度和深度图的深度大小，如果小的话说明需要上阴影，于是涂黑。这是最基础的阴影映射部分，为此我们先定义一个阴影映射所使用的帧缓冲：

```cpp
auto shadow = FrameBuffer::Builder(shadowWidth, shadowHeight)
                       .SetAttachment("shadowmap", GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_ATTACHMENT, false, GetDepthTextureParams())
                       .Build();
```

其中shadowMapFBO是阴影映射时所用到的帧缓冲，我们以光源为视角，作向量$\vec{v}=\vec{0}-\vec{sun}$，以$\vec{v}/|\vec{v}|$ 为光源视角，绘制ShadowMap（或者说Depth Map)。

在绘制的时候，我们应当启用深度检测，因为我们需要生成一张带有深度信息的深度图，具体绘制代码如下：

```cpp
void RenderMgr::ShadowPass() {
    auto ctx = Global::GetInstance();
    auto shader = ctx->GetMgr<AssetsMgr>()->GetShader("shadow");
    ctx->lightView = glm::lookAt(ctx->lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    ctx->lightSpaceMatrix = ctx->lightProjection * ctx->lightView;
    shader->use();
    shader->setMatrix4("lightSpaceMatrix", ctx->lightSpaceMatrix);

    this->shadow->Bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);
    shader->setMatrix4("model", glm::mat4(1.0f));
    ctx->GetMgr<AssetsMgr>()->GetModel<Terrain>("terrain")->Draw(*shader);
    glCullFace(GL_BACK);
    this->shadow->UnBind();
}
```

ShadowShader如下：

```glsl
#version 330 core
layout (location = 0) in vec3 aPos;

uniform int isLight;
uniform mat4 lightSpaceMatrix;
uniform mat4 cameraSpaceMatrix;
uniform mat4 model;

void main() {
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0f);
}
```

因为我们不对绘制任何片元，只是在shadowMapFBO的缓冲区下（不可见）的时候绘制深度图，所以只需要传递坐标信息即可。

其中，MVP变换矩阵中$lightSpaceMatrix$ 的值如下：

$$
\begin{aligned}
&lightSpaceMatrix=lightOrthoProjection\times lightView\\
&lightView=LookAt(\vec{sun})
\end{aligned}
$$

而投影矩阵我们采用正射投影而不是透视投影。这是因为正交投影矩阵并不会将场景用透视图进行变形，所有视线/光线都是平行的，这使它对于定向光来说是个很好的投影矩阵。然而透视投影矩阵，会将所有顶点根据透视关系进行变形，结果因此而不同。

![](https://cdn.jsdelivr.net/gh/InverseDa/image@master/image/Untitled%203.png)

而基本的阴影绘制思路如下：

```glsl
float ShadowCalculation(vec4 fragPos) {
    vec4 fragPosLightSpace = lightSpaceMatrix * fragPos;
    float bias = 0.005;
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closetDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
        float shadow = 0.0;
    if(currentDepth -bias > closetDepth) shadow = 1.0f;
    return shadow;
}
```

PCF是优化阴影的一种方法，因为这种方法所获得的阴影可能会由于ShadowMap的分辨率而存在质量问题，比如锯齿阴影。

PCF在获得阴影的颜色之后，再做加权平均。也就是说从深度贴图中多次采样，每一次采样的纹理坐标都稍有不同。每个独立的片元可能在也可能不再阴影中。将这些结果加权平均，我们就得到了PCF阴影。

```glsl
float ShadowCalculation(vec4 fragPos) {
    vec4 fragPosLightSpace = lightSpaceMatrix * fragPos;
    float bias = 0.005;
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closetDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - 0.0002 > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    if (projCoords.z > 1.0) shadow = 0.0;

    return shadow;
}
```

其中textureSize返回一个shadowMap的0级mipmap的vec2类型的宽和高（Lod）。用1除以它返回一个单独纹理像素的大小，我们用以对纹理坐标进行偏移，确保每个新样本，来自不同的深度值。这里我们采样得到9个值，它们在投影坐标的x和y值的周围，

为阴影阻挡进行测试，并最终通过样本的总数目将结果平均化。

![](https://cdn.jsdelivr.net/gh/InverseDa/image@master/image/Untitled%204.png)

### 4. 环境贴图

环境贴图用于构建天空盒，这样可以极大的提高场景的美观。需要注意的是，环境贴图最好是静态的，另外我们希望天空是“触不可及”的，所以需要去除位移因素，使得天空永远相对于人不动。

```cpp
void Cube::VerticesSetup(const std::string& jsonPath) {
    this->vertices = {
        glm::vec3(-1.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, -1.0f, 1.0f),
        glm::vec3(-1.0f, -1.0f, 1.0f),
        glm::vec3(-1.0f, 1.0f, -1.0f),
        glm::vec3(1.0f, 1.0f, -1.0f),
        glm::vec3(1.0f, -1.0f, -1.0f),
        glm::vec3(-1.0f, -1.0f, -1.0f)
    };
   this->indices = {
        0, 1, 2,
        0, 2, 3,
        1, 5, 6,
        1, 6, 2,
        5, 4, 7,
        5, 7, 6,
        4, 0, 3,
        4, 3, 7,
        4, 5, 1,
        4, 1, 0,
        3, 2, 6,
        3, 6, 7
    };
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
    glGenBuffers(1, &this->EBO);

    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

    glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(glm::vec3), this->vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned int), this->indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
```

创建天空盒需要用到正方体，所以最好我们能够手动输入坐标、法向量和UV坐标。

另外还需要加载环境贴图：

```cpp
unsigned int loadSkyBox(std::vector<std::string> &faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrComponents;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cout << "Error: Fail to load skybox !\n";
            stbi_image_free(data);
            exit(0);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return textureID;
}
```

### 5. 动态水

水我们可以简单理解为一个平面，这个平面我们可以细分成很多小的三角面片，这样我们就可以随意操控这个平面内的任意遵循三角面片关系下的坐标。

对于细分部分，我们由下面的代码可以解决：

```cpp
void Water::VerticesSetup() {
    std::pair<int, int> textureCoords[] = {{0, 0}, {1, 0}};
    int textureIndex = 0;

    for (int i = 0; i < this->width - 1; i++) {
        for (int j = 0; j < this->height - 1; j++) {
            int index = i * this->width + j;
            glm::vec3 position = glm::vec3(static_cast<float>(i), 0.0f, static_cast<float>(j));
            glm::vec2 texcoords = glm::vec2(static_cast<float>(textureCoords[textureIndex].first),
                                            static_cast<float>(textureCoords[textureIndex].second));
            this->vertices.push_back({position, texcoords});
            textureIndex = (textureIndex + 1) % 2;

            this->indices.push_back(index);
            this->indices.push_back(index + 1);
            this->indices.push_back((i + 1) * this->width + j);
            this->indices.push_back(index + 1);
            this->indices.push_back((i + 1) * this->width + j);
            this->indices.push_back((i + 1) * this->width + j + 1);
        }
    }
    // vao
    glGenVertexArrays(1, &this->VAO);
    glBindVertexArray(this->VAO);
    // vbo
    glGenBuffers(1, &this->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(std::pair<glm::vec3, glm::vec2>), this->vertices.data(), GL_STATIC_DRAW);
    // attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(std::pair<glm::vec3, glm::vec2>), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(std::pair<glm::vec3, glm::vec2>), (void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(1);
    // ebo
    glGenBuffers(1, &this->EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned int), this->indices.data(), GL_STATIC_DRAW);
    // unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
```

我们根据WATER_VERTICES_HEIGHT_AND_WIDTH，创建了WATER_VERTICES_HEIGHT_AND_WIDTH * WATER_VERTICES_HEIGHT_AND_WIDTH的水矩阵，每个矩阵的元素都是一个顶点。对于UV的计算，我们是采用hashMapx来进行分配，这样保证水面中每个小正方形（两个三角面片）能显示一张水贴图。（后来发现，因为我们要用环境贴图反射的颜色来覆盖水面，那么这个UV就没啥必要了……）

绘制代码如下，因为使用索引存储，所以绘制代码很简单：

```cpp
    glBindVertexArray(waterVAO);
    glDrawElements(GL_TRIANGLES,
                   static_cast<int>(waterIndices.size()),
                   GL_UNSIGNED_INT,
                   nullptr);
```

#### (1) 利用最简单的波函数创建动态水

一个简单的波函数由三角函数叠加：

$$
y(x,z,t)=A\sin(\omega_1 x + \lambda _1t)+B\cos(\omega_2 z + \lambda_2 t)
$$

其中$x,y,z$ 为水面每个三角面片的坐标，对于单个顶点坐标而言，这个方程表达了水顶点的高度$y$ 随水平方向两个坐标$x,z$ 和世界时间$t$ 的具体关系。

我们可以用顶点着色器实现：

```cpp
#version 330 core
in vec3 aPos;
in vec2 texCoord;

out vec2 aTexcoord;
out VS_OUT{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
    vec4 positionInViewCoord;
}vs_out;
out vec4 clipSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float worldTime;

void main() {
    // getBump
    vec4 worldPosition = model * vec4(aPos, 1.0f);
    worldPosition.y += 0.2 * (sin(worldPosition.z * 4 + worldTime * 1.5)  + cos(worldPosition.x * 4 + worldTime * 1.5));
    vs_out.FragPos = worldPosition.xyz;
    vs_out.positionInViewCoord = projection * worldPosition;
    vs_out.Normal = vec3(0, 1, 0);
    vs_out.TexCoords = texCoord;
    gl_Position = projection *  view * worldPosition;
    clipSpace = projection *  view * worldPosition;
}
```

其中worldPosition就是顶点的世界坐标，debug模式中能清晰看到波动效果。

![](https://cdn.jsdelivr.net/gh/InverseDa/image@master/image/Untitled%205.png)

#### (2) 噪声图生成水波

为了方便期间，我们可以使用现成的柏林噪声图，将其导入：

![](https://cdn.jsdelivr.net/gh/InverseDa/image@master/image/Untitled%206.png)

导入后可见：

```cpp
float getWave(vec3 pos) {
    float speed1 = worldTime * 85 / (noiseTextureResolution * 15);
    vec3 coord1 = pos.xyz / noiseTextureResolution;
    coord1.x *= 3;
    coord1.x += speed1;
    coord1.z += speed1 * 0.2;
    float noise1 = texture(noisetex, coord1.xz).x;

    float speed2 = worldTime * 85 / (noiseTextureResolution * 7);
    vec3 coord2 = pos.xyz / noiseTextureResolution;
    coord2.x *= 0.5;
    coord2.x -= speed2 * 0.15 + noise1 * 0.05;
    coord2.z -= speed2 * 0.7 - noise1 * 0.05;
    float noise2 = texture(noisetex, coord2.xz).x;

    return noise2 * 0.6 + 0.4;;
}
```

将波纹合成到最终颜色里：

```cpp
    vec3 tex = vec3(texture(waterTexture, fs_in.TexCoords));
    float wave = getWave(fs_in.FragPos);
    vec3 color = tex * wave;
```

#### (3) 环境映射

我们可以给水增加环境贴图的颜色，这样会显著提高真实感。由于天空盒的环境贴图UV是三维的，所以可以用三维的反射向量来取色，另外，可以让法向量随着波纹扰动，这样映射后的颜色也是来回变化的：

```glsl
vec4 reflectSky(float wave) {
    vec3 newNormal = fs_in.Normal;
    newNormal.z += 0.05 * (((wave - 0.4) / 0.6) * 2 - 1);
    newNormal = normalize(newNormal);
    vec3 I = normalize(fs_in.FragPos - viewPos);
    vec3 R = reflect(I, normalize(newNormal));
    vec4 skyColor = vec4(texture(skyBox, R).rgb, 1.0);
    return skyColor;
}

...
main:
    vec4 skyColor = reflectSky(wave);
    vec3 final = skyColor.rgb;
```

直接拿来使用，就可以得到相对真实的水：

![](https://cdn.jsdelivr.net/gh/InverseDa/image@master/image/Untitled%207.png)

![](https://cdn.jsdelivr.net/gh/InverseDa/image@master/image/Untitled%208.png)

#### (4) Screen Space Reflection

屏幕空间反射是一种后处理算法，这个算法需要在延迟渲染管线中执行，常用于写水面的反射。

其思路是在后处理的过程中，所有片元都在相机空间下运算颜色，并且最后在NDC空间内取色（充当UV）。

取水面某个世界坐标点P，将P变换到相机坐标下：

$$
\vec{P'}=View\times\vec{P}
$$

将$\vec{P'}$ 单位化，就可以得到从相机（人）到该点的相机空间下的入射向量，记作$T=\vec{P'}/|\vec{P}|$。根据该顶点的法向量，可以得到反射向量$\vec{T'}$。

以水面的顶点为起点，往反射向量的方向叠加。如果当前顶点碰撞到了世界的片元，那么就算反射成功，取碰撞点的颜色，赋值给水面。

```glsl
    float maxRayDistance = 100.0f;
    float wave = getWave(worldPosFromDepth(depth))  
    // View Space ray calculation
    vec3 pixelPositionTexture;
    pixelPositionTexture.xy = vec2(gl_FragCoord.x / SCR_WIDTH,
                                   gl_FragCoord.y / SCR_HEIGHT);
    vec3 normalView = texture(Normal, pixelPositionTexture.xy).rgb;
    // 增加法线扰动
    normalView.z += 0.05 * (((wave - 0.4) / 0.6) * 2 - 1);
    // trans to View space
    normalView = mat3(view) * normalView;
    float pixelDepth = texture(DepthTex, pixelPositionTexture.xy).r;
    pixelPositionTexture.z = pixelDepth;
    vec4 positionView = inverseP * vec4(pixelPositionTexture * 2 - 1, 1);
    positionView /= positionView.w;
    vec3 reflectionView = normalize(reflect(positionView.xyz, normalView));
    if(reflectionView.z > 0) {
        FragColor = water;
        return;
    }
    vec3 rayEndPositionView = positionView.xyz + reflectionView * maxRayDistance    
    // UV Space ray calculation
    vec4 rayEndPositionTexture = projection * vec4(rayEndPositionView, 1);
    rayEndPositionTexture /= rayEndPositionTexture.w;
    rayEndPositionTexture.xyz = rayEndPositionTexture.xyz * 0.5 + 0.5;
    vec3 rayDirectionTexture = rayEndPositionTexture.xyz - pixelPositionTexture 
    ivec2 screenSpaceStartPosition = ivec2(pixelPositionTexture.x * SCR_WIDTH,
                                           pixelPositionTexture.y * SCR_WIDTH);
    ivec2 screenSpaceEndPosition = ivec2(rayEndPositionTexture.x * SCR_WIDTH,
                                         rayEndPositionTexture.y * SCR_WIDTH);
    ivec2 screenSpaceDistance = screenSpaceEndPosition - screenSpaceStartPosition;
    int screenSpaceMaxDistance = max(abs(screenSpaceDistance.x), abs(screenSpaceDistance.y)) / 2;
    rayDirectionTexture /= max(screenSpaceMaxDistance, 0.001f)  
    // tracing
    vec3 outColor = rayTrace(pixelPositionTexture, rayDirectionTexture, screenSpaceMaxDistance);
    if(outColor == vec3(0.0f)) FragColor = water;
    else {
        outColor = outColor;
        FragColor = vec4(outColor, 1.0);
    }
    return;
```

而rayMarch主体：

```glsl
vec3 rayTrace(vec3 rayPos, vec3 dir, int iterationCount) {
    float sampleDepth;
    vec3 hitColor = vec3(0.0f);
    bool hit = false;

    for(int i = 0; i < 256; i++) {
        rayPos += dir;
        if(rayIsOutofScreen(rayPos.xy)) {
            break;
        }
        sampleDepth = texture(DepthTex, rayPos.xy).r;
        float depthDif = rayPos.z - sampleDepth;
        if(depthDif >= 0 && depthDif < 0.00001) {
            hit = true;
            hitColor = texture(DiffuseSpecular, rayPos.xy).rgb;
            break;
        }
    }
    return hitColor;
}
```
最终可以看到结果：
![](https://cdn.jsdelivr.net/gh/InverseDa/image@master/image/Untitled%209.png)
