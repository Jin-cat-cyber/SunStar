#include "ForwardPbr.h"

void ForwardPbr::Draw(const ForwardSceneInfo& Info, Shader& shader)
{
    //shader.use();
    //shader.setMat4("projection", projection);
    //shader.setMat4("view", view);
    //shader.setMat4("model", spaceshipModel);
    //shader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(spaceshipModel))));
    //shader.setVec3("camPos", camera.Position);

    //shader.setVec3("lightPositions[0]", pointSunPositions);
    //shader.setVec3("lightColors[0]", glm::vec3(200.0f, 200.0f, 160.0f));
    //shader.setFloat("lightConstant", 1.0f);
    //shader.setFloat("lightLinear", 0.0002f);
    //shader.setFloat("lightQuadratic", 0.000005f);

    //shader.setBool("useMetallicMap", true);
    //shader.setBool("useRoughnessMap", true);
    //shader.setFloat("metallicValue", 0.0f);
    //shader.setFloat("roughnessValue", 0.5f);
    //shader.setBool("useEmissiveMap", true);
    //shader.setBool("useAOMap", false);
    //shader.setFloat("aoValue", 1.0f);
    //shader.setFloat("emissiveStrength", 2.0f);
    //// 菲涅尔
    //shader.setVec3("rimColor", glm::vec3(0.4f, 0.6f, 1.0f));  // 淡蓝 ( > 1.0 时会参与到Bloom)
    //shader.setFloat("rimPower", 3.0f);                        // 越大边缘越锐利
    //shader.setFloat("rimStrength", 0.6f);                     // 强度 

    //shader.setBool("shadows", true);
    //shader.setBool("PCSS", false);
    //shader.setFloat("far_plane", shadow_far);
}

void ForwardPbr::DrawDepth(Shader& shader)
{
    shader.setBool("instanced", false);
    shader.setMat4("model", GetModelMatrix());
    model.Draw(shader);
}

glm::mat4 ForwardPbr::GetModelMatrix() const
{
    glm::mat4 m = glm::mat4(1.0f);
    m = glm::translate(m, position);
    m = glm::rotate(m, glm::radians(rotation.x), glm::vec3(1, 0, 0));
    m = glm::rotate(m, glm::radians(rotation.y), glm::vec3(0, 1, 0));
    m = glm::rotate(m, glm::radians(rotation.z), glm::vec3(0, 0, 1));
    m = glm::scale(m, scale);
    return m;
}
