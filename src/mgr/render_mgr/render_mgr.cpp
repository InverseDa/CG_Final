#include "mgr/render_mgr/render_mgr.hpp"

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
    this->gbuffer = GBuffer::CreateGBuffer(width, height);
    this->composite = FrameBuffer::CreateFrameBuffer(width, height);
    this->shadow = ShadowBuffer::CreateShadowBuffer(width, height);
}

void RenderMgr::Run() {
    this->GBufferPass();
    this->LightPass();
    this->ShadowPass();
    this->CompositePass();
}

void RenderMgr::CompositePass() {
}

void RenderMgr::GBufferPass() {
    // this->gbuffer->Bind();
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // // Terrain
    // glm::mat4 model = glm::mat4(1.0f);
    // const std::shared_ptr<Shader> terrainShader = Global::GetInstance()->GetMgr<AssetsMgr>()->GetShader("g_terrain");
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

void RenderMgr::LightPass() {
}

void RenderMgr::ShadowPass() {
    this->shadow->Bind();
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
