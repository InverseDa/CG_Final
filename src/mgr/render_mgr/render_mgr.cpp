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
    this->ResizeCallback();
}

void RenderMgr::Run() {
    this->ShadowPass();
    this->GBufferPass();
    this->CompositePass();
    this->FinalPass();
}

void RenderMgr::ResizeCallback() {
    auto ctx = Global::GetInstance();
    auto width = ctx->window->getWidth();
    auto height = ctx->window->getHeight();
    auto shadowWidth = 1024;
    auto shadowHeight = 1024;

    this->shadow = FrameBuffer::Builder(shadowWidth, shadowHeight)
                       .SetAttachment("shadowmap", GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_ATTACHMENT, READ_BUFFER_FLAG, GetDepthTextureParams())
                       .Build();
    this->gbuffer = FrameBuffer::Builder(width, height)
                        .SetAttachment("position", GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT0, DRAW_BUFFER_FLAG)
                        .SetAttachment("normal", GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT1, DRAW_BUFFER_FLAG)
                        .SetAttachment("albedoSpec", GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT2, DRAW_BUFFER_FLAG)
                        .SetAttachment("water", GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT3, DRAW_BUFFER_FLAG)
                        .SetAttachment("depth", GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_ATTACHMENT, READ_BUFFER_FLAG, GetDepthTextureParams())
                        .Build();
    this->composite1 = FrameBuffer::Builder(width, height)
                           .SetAttachment("position", GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT0, DRAW_BUFFER_FLAG)
                           .SetAttachment("normal", GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT1, DRAW_BUFFER_FLAG)
                           .SetAttachment("albedoSpec", GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT2, DRAW_BUFFER_FLAG)
                           .SetAttachment("water", GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT3, DRAW_BUFFER_FLAG)
                           .SetAttachment("depth", GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_ATTACHMENT, READ_BUFFER_FLAG, GetDepthTextureParams())
                           .Build();
}

void RenderMgr::CompositePass() {
    auto ctx = Global::GetInstance();
    auto cameraMgr = ctx->GetMgr<CameraMgr>();
    auto assetsMgr = ctx->GetMgr<AssetsMgr>();

    auto shader = assetsMgr->GetShader("composite1");

    composite1->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gbuffer->GetTexture("position"));
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gbuffer->GetTexture("normal"));
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gbuffer->GetTexture("albedoSpec"));
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gbuffer->GetTexture("depth"));
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, gbuffer->GetTexture("water"));
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, shadow->GetTexture("shadowmap"));
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, assetsMgr->GetTexture("perlin_noise")->getId());
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
    composite1->UnBind();
}

void RenderMgr::GBufferPass() {
    auto ctx = Global::GetInstance();
    auto cameraMgr = ctx->GetMgr<CameraMgr>();
    auto assetsMgr = ctx->GetMgr<AssetsMgr>();

    auto terrainShader = assetsMgr->GetShader("g_terrain");
    auto waterShader = assetsMgr->GetShader("g_water");
    auto skyboxShader = assetsMgr->GetShader("skybox");

    this->gbuffer->Bind();
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
    assetsMgr->GetModel<Terrain>("terrain")->Draw(*terrainShader);

    // water
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, assetsMgr->GetTexture("skybox")->getId());
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, assetsMgr->GetTexture("perlin_noise")->getId());
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, shadow->GetTexture("shadowmap"));
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, assetsMgr->GetTexture("terrain_diffuse")->getId());
    model = glm::translate(glm::mat4(1.0f), glm::vec3(-1024.0, 0.00000001f, -1024.0f));
    model = glm::scale(model, glm::vec3(5.0));
    waterShader->use();
    waterShader->setMatrix4("model", model);
    waterShader->setMatrix4("view", cameraMgr->GetViewMatrix());
    waterShader->setMatrix4("projection", cameraMgr->GetProjectionMatrix());
    waterShader->setFloat("worldTime", static_cast<float>(glfwGetTime()));
    waterShader->setInt("waterTexture", 0);
    waterShader->setInt("skyBox", 1);
    waterShader->setInt("noisetex", 2);
    waterShader->setInt("shadowMap", 3);
    waterShader->setInt("tex", 4);
    waterShader->setVector3("lightPos", ctx->lightPos);
    waterShader->setVector3("viewPos", cameraMgr->GetCameraPosition());
    waterShader->setFloat("viewHeight", ctx->window->getHeight());
    waterShader->setFloat("viewWidth", ctx->window->getWidth());
    assetsMgr->GetModel<Water>("water")->Draw(*waterShader);

    // skybox
    skyboxShader->use();
    skyboxShader->setMatrix4("view", glm::mat4(glm::mat3(cameraMgr->GetViewMatrix())));
    skyboxShader->setMatrix4("projection", cameraMgr->GetProjectionMatrix());
    skyboxShader->setInt("skyBox", 0);
    assetsMgr->GetModel<Cube>("skybox")->Draw(*skyboxShader);
    this->gbuffer->UnBind();
}

void RenderMgr::FinalPass() {
    auto ctx = Global::GetInstance();
    auto cameraMgr = ctx->GetMgr<CameraMgr>();
    auto assetsMgr = ctx->GetMgr<AssetsMgr>();

    auto shader = assetsMgr->GetShader("composite2");

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader->use();
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, composite1->GetTexture("position"));
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, composite1->GetTexture("albedoSpec"));
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gbuffer->GetTexture("depth"));
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, composite1->GetTexture("water"));
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, composite1->GetTexture("normal"));
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, assetsMgr->GetTexture("perlin_noise")->getId());
    shader->setInt("Position", 1);
    shader->setInt("DiffuseSpecular", 2);
    shader->setInt("DepthTex", 3);
    shader->setInt("WaterTex", 4);
    shader->setInt("Normal", 5);
    shader->setInt("noisetex", 6);
    shader->setFloat("near", cameraMgr->GetNear());
    shader->setFloat("far", cameraMgr->GetFar());
    shader->setFloat("worldTime", static_cast<float>(glfwGetTime()));
    shader->setMatrix4("view", cameraMgr->GetViewMatrix());
    shader->setMatrix4("inverseV", cameraMgr->GetInverseViewMatrix());
    shader->setMatrix4("projection", cameraMgr->GetProjectionMatrix());
    shader->setMatrix4("inverseP", cameraMgr->GetInverseProjectionMatrix());
    shader->setMatrix4("inverseVP", cameraMgr->GetInverseViewProjectionMatrix());
    shader->setMatrix4("lightSpaceMatrix", ctx->lightSpaceMatrix);
    shader->setVector3("lightPos", ctx->lightPos);
    shader->setVector3("viewPos", cameraMgr->GetCameraPosition());
    shader->setVector3("lightColor", ctx->lightColor);
    shader->setVector3("lightDirection", glm::vec3(0) - ctx->lightPos);
    shader->setInt("SCR_WIDTH", ctx->window->getWidth());
    shader->setInt("SCR_HEIGHT", ctx->window->getHeight());
    screen.Draw(*shader);
}

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
