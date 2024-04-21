#include "mgr/render_mgr/render_mgr.hpp"
#include "model/triangle.hpp"
#include "model/water.hpp"

static std::shared_ptr<RenderMgr> instance = nullptr;
static std::once_flag singletonFlag;

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
    this->gbuffer = FrameBuffer::Builder(width, height)
                        .SetAttachment("position", GL_RGB16F, GL_RGB, GL_FLOAT, GL_COLOR_ATTACHMENT0)
                        .SetAttachment("normal", GL_RGB16F, GL_RGB, GL_FLOAT, GL_COLOR_ATTACHMENT1)
                        .SetAttachment("albedoSpec", GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT2)
                        .SetAttachment("depth", GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_ATTACHMENT)
                        .Build();
    this->composite1 = FrameBuffer::Builder(width, height)
                           .SetAttachment("position", GL_RGB16F, GL_RGB, GL_FLOAT, GL_COLOR_ATTACHMENT0)
                           .SetAttachment("normal", GL_RGB16F, GL_RGB, GL_FLOAT, GL_COLOR_ATTACHMENT1)
                           .SetAttachment("albedoSpec", GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT2)
                           .SetAttachment("depth", GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_ATTACHMENT)
                           .Build();
}

void RenderMgr::Run() {
    this->GBufferPass();
    this->LightPass();
    this->ShadowPass();
    this->CompositePass();
}

void RenderMgr::CompositePass() {
    //    auto ctx = Global::GetInstance();
    //    auto cameraMgr = ctx->GetMgr<CameraMgr>();
    //    composite1->Bind();
    //    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //    auto shader = ctx->GetMgr<AssetsMgr>()->GetShader("composite1");
    //    shader->use();
    //    glActiveTexture(GL_TEXTURE0);
    //    glBindTexture(GL_TEXTURE_2D, gbuffer->getPosition());
    //    glActiveTexture(GL_TEXTURE1);
    //    glBindTexture(GL_TEXTURE_2D, gbuffer->getNormal());
    //    glActiveTexture(GL_TEXTURE2);
    //    glBindTexture(GL_TEXTURE_2D, gbuffer->getAlbedoSpec());
    //    shader->setInt("gPosition", 0);
    //    shader->setInt("gNormal", 1);
    //    shader->setInt("gDiffuseSpecular", 2);
    //    shader->setFloat("near", cameraMgr->GetNear());
    //    shader->setFloat("far", cameraMgr->GetFar());
    //    shader->setFloat("worldTime", static_cast<float>(glfwGetTime()));
    //    shader->setMatrix4("view", cameraMgr->GetViewMatrix());
    //    shader->setMatrix4("viewInverse", glm::inverse(cameraMgr->GetViewMatrix()));
    //    shader->setMatrix4("projection", cameraMgr->GetProjectionMatrix());
    //    shader->setMatrix4("projectionInverse", glm::inverse(cameraMgr->GetProjectionMatrix()));
    //    //    shader->setMatrix4("lightSpaceMatrix", lightSpaceMatrix);
    //    //    shader->setVector3("lightPos", lightPos);
    //    //    shader->setVector3("viewPos", camera.cameraPos);
    //    //    shader->setVector3("lightColor", lightColor);
    //    //    shader->setVector3("lightDirection", glm::vec3(0) - lightPos);
    //    screen.Draw(*shader);
    //    composite1->Unbind();
}

void RenderMgr::GBufferPass() {
    auto ctx = Global::GetInstance();
    auto cameraMgr = ctx->GetMgr<CameraMgr>();
//    this->gbuffer->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Terrain
    glm::mat4 model = glm::mat4(1.0f);
    const std::shared_ptr<Shader> terrainShader = ctx->GetMgr<AssetsMgr>()->GetShader("g_terrain");
    terrainShader->use();
    terrainShader->setMatrix4("model", model);
    terrainShader->setMatrix4("view", ctx->GetMgr<CameraMgr>()->GetViewMatrix());
    terrainShader->setMatrix4("projection", ctx->GetMgr<CameraMgr>()->GetProjectionMatrix());
    terrainShader->setInt("tex", 0);
    terrainShader->setInt("specular", 1);
    terrainShader->setInt("normal", 2);
    ctx->GetMgr<AssetsMgr>()->GetModel<Terrain>("terrain")->Draw(*terrainShader);

    // Nano
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    const std::shared_ptr<Shader> assimpShader = ctx->GetMgr<AssetsMgr>()->GetShader("g_model");
    assimpShader->use();
    assimpShader->setMatrix4("model", model);
    assimpShader->setMatrix4("view", ctx->GetMgr<CameraMgr>()->GetViewMatrix());
    assimpShader->setMatrix4("projection", ctx->GetMgr<CameraMgr>()->GetProjectionMatrix());
    ctx->GetMgr<AssetsMgr>()->GetModel<AssimpModel>("nanosuit")->Draw(*assimpShader);

    // draw triangle
    const std::shared_ptr<Shader> triangleShader = ctx->GetMgr<AssetsMgr>()->GetShader("triangle");
    triangleShader->use();
    triangleShader->setMatrix4("model", model);
    triangleShader->setMatrix4("view", cameraMgr->GetViewMatrix());
    triangleShader->setMatrix4("projection", cameraMgr->GetProjectionMatrix());
    ctx->GetMgr<AssetsMgr>()->GetModel<Triangle>("triangle")->Draw(*triangleShader);

//    this->gbuffer->Unbind();
}

void RenderMgr::LightPass() {
}

void RenderMgr::ShadowPass() {
    //    this->shadow->Bind();
    // lightProjection = glm::ortho(-1000.0f, 1000.0f, -1000.0f, 1000.0f, 1.0f, 100000.0f);
    // //    lightProjection = glm::perspective(glm::radians(camera.fov),
    // //    WINDOW_WIDTH * 1.0f / WINDOW_HEIGHT, 0.1f, 100000.0f);
    // lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    // lightSpaceMatrix = lightProjection * lightView;
    // shadowShader->use();
    // shadowShader->set4Matrix("lightSpaceMatrix", lightSpaceMatrix);
    // const std::shared_ptr<Shader> shadowShader = Global::GetInstance()->GetMgr<AssetsMgr>()->GetShader("shadow");
    // // Terrain
    // glm::mat4 model = glm::mat4(1.0f);
    // terrainShader->setMatrix4("model", model);
    // terrainShader->setMatrix4("view", Global::GetInstance()->GetMgr<CameraMgr>()->GetViewMatrix());
    // terrainShader->setMatrix4("projection", Global::GetInstance()->GetMgr<CameraMgr>()->GetProjectionMatrix());
    // terrainShader->setInt("tex", 0);
    // terrainShader->setInt("specular", 1);
    // terrainShader->setInt("normal", 2);
    // Global::GetInstance()->GetMgr<AssetsMgr>()->GetModel<Terrain>("terrain")->Draw(*terrainShader);
    //
    // // Nano
    // model = glm::translate(glm::mat4(1.0f), glm::vec3(800.0f, 30.0f, 500.0f));
    // model = glm::scale(model, glm::vec3(1.0f));
    // const std::shared_ptr<Shader> assimpShader = Global::GetInstance()->GetMgr<AssetsMgr>()->GetShader("g_terrain");
    // assimpShader->use();
    // assimpShader->setMatrix4("model", model);
    // assimpShader->setMatrix4("view", Global::GetInstance()->GetMgr<CameraMgr>()->GetViewMatrix());
    // assimpShader->setMatrix4("projection", Global::GetInstance()->GetMgr<CameraMgr>()->GetProjectionMatrix());
    // Global::GetInstance()->GetMgr<AssetsMgr>()->GetModel<AssimpModel>("nanosuit")->Draw(*assimpShader);
    //
    // this->gbuffer->Unbind();
}
