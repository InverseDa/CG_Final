#include "mgr/render_mgr/render_mgr.hpp"
#include "model/triangle.hpp"
#include "model/water.hpp"

static std::shared_ptr<RenderMgr> instance = nullptr;
static std::once_flag singletonFlag;

std::vector<std::pair<GLenum, GLfloat>> RenderMgr::GetDepthTextureParams() {
    return {
        {GL_TEXTURE_MIN_FILTER, GL_NEAREST},
        {GL_TEXTURE_MAG_FILTER, GL_NEAREST},
        {GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
        {GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE},
    };
}

std::shared_ptr<RenderMgr> RenderMgr::GetInstance() {
    std::call_once(singletonFlag, [&] {
        instance = std::make_shared<RenderMgr>();
    });
    return instance;
}

RenderMgr::RenderMgr() {
    // 创建所有需要的Bufer
    const int width = JsonConfigLoader::Read("env/settings.json", "width");
    const int height = JsonConfigLoader::Read("env/settings.json", "height");
    const int shadowWidth = 1024;
    const int c = 1024;
    this->shadow = FrameBuffer::Builder(shadowWidth, shadowWidth)
                       .SetAttachment("shadowmap", GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_ATTACHMENT, false, GetDepthTextureParams())
                       .Build();
    this->gbuffer = FrameBuffer::Builder(width, height)
                        .SetAttachment("position", GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT0)
                        .SetAttachment("normal", GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT1)
                        .SetAttachment("albedoSpec", GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT2)
                        .SetAttachment("water", GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT3)
                        .SetAttachment("depth", GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_ATTACHMENT, false, GetDepthTextureParams())
                        .Build();
    this->composite1 = FrameBuffer::Builder(width, height)
                           .SetAttachment("position", GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT0)
                           .SetAttachment("normal", GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT1)
                           .SetAttachment("albedoSpec", GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT2)
                           .SetAttachment("water", GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT3)
                           .SetAttachment("depth", GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_ATTACHMENT, false, GetDepthTextureParams())
                           .Build();
}

void RenderMgr::Run() {
    this->ShadowPass();
    this->GBufferPass();
    this->CompositePass();
}

void RenderMgr::CompositePass() {
    auto ctx = Global::GetInstance();
    auto cameraMgr = ctx->GetMgr<CameraMgr>();
    auto assetsMgr = ctx->GetMgr<AssetsMgr>();

    auto shader = assetsMgr->GetShader("composite1");

    composite1->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gbuffer->getTexture("position"));
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gbuffer->getTexture("normal"));
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gbuffer->getTexture("albedoSpec"));
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gbuffer->getTexture("depth"));
    //    glActiveTexture(GL_TEXTURE4);
    //    glBindTexture(GL_TEXTURE_2D, shadow->getTexture("water"));
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, shadow->getTexture("shadowmap"));
    //    glActiveTexture(GL_TEXTURE6);
    //    glBindTexture(GL_TEXTURE_2D, assetsMgr->GetTexture("noise")->getId());
    shader->setInt("gPosition", 0);
    shader->setInt("gNormal", 1);
    shader->setInt("gDiffuseSpecular", 2);
    shader->setInt("gDepthTex", 3);
    shader->setInt("gWaterTex", 4);
    shader->setInt("shadowMap", 5);
    shader->setInt("noisetex", 6);
    shader->setFloat("near", cameraMgr->GetNear());
    shader->setFloat("far", cameraMgr->GetFar());
    shader->setFloat("worldTime", static_cast<float>(glfwGetTime()));
    shader->setMatrix4("view", cameraMgr->GetViewMatrix());
    shader->setMatrix4("viewInverse", glm::inverse(cameraMgr->GetViewMatrix()));
    shader->setMatrix4("projection", cameraMgr->GetProjectionMatrix());
    shader->setMatrix4("projectionInverse", glm::inverse(cameraMgr->GetProjectionMatrix()));
    shader->setMatrix4("lightSpaceMatrix", ctx->lightSpaceMatrix);
    shader->setVector3("lightPos", ctx->lightPos);
    shader->setVector3("viewPos", cameraMgr->GetCameraPosition());
    shader->setVector3("lightColor", ctx->lightColor);
    shader->setVector3("lightDirection", glm::vec3(0) - ctx->lightPos);
    screen.Draw(*shader);
    composite1->unbind();

    // composite2 phase
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    auto shader2 = assetsMgr->GetShader("composite2");
    shader2->use();
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, composite1->getTexture("position"));
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, composite1->getTexture("albedoSpec"));
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gbuffer->getTexture("depth"));
    //    glActiveTexture(GL_TEXTURE4);
    //    glBindTexture(GL_TEXTURE_2D, composite1->getTexture("water"));
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, composite1->getTexture("normal"));
    //    glActiveTexture(GL_TEXTURE6);
    //    glBindTexture(GL_TEXTURE_2D, assetsMgr->GetTexture("noise")->getId());
    shader2->setInt("Position", 1);
    shader2->setInt("DiffuseSpecular", 2);
    shader2->setInt("DepthTex", 3);
    shader2->setInt("WaterTex", 4);
    shader2->setInt("Normal", 5);
    shader2->setInt("noisetex", 6);
    shader2->setFloat("near", cameraMgr->GetNear());
    shader2->setFloat("far", cameraMgr->GetFar());
    shader2->setFloat("worldTime", static_cast<float>(glfwGetTime()));
    shader2->setMatrix4("view", cameraMgr->GetViewMatrix());
    shader2->setMatrix4("inverseV", glm::inverse(cameraMgr->GetViewMatrix()));
    shader2->setMatrix4("projection", cameraMgr->GetProjectionMatrix());
    shader2->setMatrix4("inverseP", glm::inverse(cameraMgr->GetProjectionMatrix()));
    shader2->setMatrix4("inverseVP", glm::inverse(cameraMgr->GetProjectionMatrix() * cameraMgr->GetViewMatrix()));
    shader2->setMatrix4("lightSpaceMatrix", ctx->lightSpaceMatrix);
    shader2->setVector3("lightPos", ctx->lightPos);
    shader2->setVector3("viewPos", cameraMgr->GetCameraPosition());
    shader2->setVector3("lightColor", ctx->lightColor);
    shader2->setVector3("lightDirection", glm::vec3(0) - ctx->lightPos);
#ifdef __APPLE__
    shader2->setInt("SCR_WIDTH", WINDOW_WIDTH * 2);
    shader2->setInt("SCR_HEIGHT", WINDOW_HEIGHT * 2);
#else
    shader2->setInt("SCR_WIDTH", 1980);
    shader2->setInt("SCR_HEIGHT", 1080);
#endif
    screen.Draw(*shader2);
}

void RenderMgr::GBufferPass() {
    auto ctx = Global::GetInstance();
    auto cameraMgr = ctx->GetMgr<CameraMgr>();
    auto assetsMgr = ctx->GetMgr<AssetsMgr>();

    auto terrainShader = assetsMgr->GetShader("g_terrain");
    auto triangleShader = ctx->GetMgr<AssetsMgr>()->GetShader("triangle");
    auto waterShader = ctx->GetMgr<AssetsMgr>()->GetShader("water");
    auto skyboxShader = ctx->GetMgr<AssetsMgr>()->GetShader("skybox");

    this->gbuffer->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Terrain
    glm::mat4 model = glm::mat4(1.0f);
    terrainShader->use();
    terrainShader->setMatrix4("model", model);
    terrainShader->setMatrix4("view", cameraMgr->GetViewMatrix());
    terrainShader->setMatrix4("projection", cameraMgr->GetProjectionMatrix());
    terrainShader->setInt("tex", 0);
    terrainShader->setInt("specular", 1);
    terrainShader->setInt("normal", 2);
    ctx->GetMgr<AssetsMgr>()->GetModel<Terrain>("terrain")->Draw(*terrainShader);
    // water

    // skybox
    skyboxShader->use();
    skyboxShader->setMatrix4("view", glm::mat4(glm::mat3(cameraMgr->GetViewMatrix())));
    skyboxShader->setMatrix4("projection", cameraMgr->GetProjectionMatrix());
    skyboxShader->setInt("skyBox", 0);
    ctx->GetMgr<AssetsMgr>()->GetModel<Cube>("skybox")->Draw(*skyboxShader);
    this->gbuffer->unbind();
}

void RenderMgr::LightPass() {
}

void RenderMgr::ShadowPass() {
    auto ctx = Global::GetInstance();
    auto shader = ctx->GetMgr<AssetsMgr>()->GetShader("shadow");
    ctx->lightView = glm::lookAt(ctx->lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    ctx->lightSpaceMatrix = ctx->lightProjection * ctx->lightView;
    shader->use();
    shader->setMatrix4("lightSpaceMatrix", ctx->lightSpaceMatrix);

    this->shadow->bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);
    shader->setMatrix4("model", glm::mat4(1.0f));
    ctx->GetMgr<AssetsMgr>()->GetModel<Terrain>("terrain")->Draw(*shader);
    glCullFace(GL_BACK);
    this->shadow->unbind();
}
