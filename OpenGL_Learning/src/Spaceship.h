#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

// 三种视角模式
enum ViewMode
{
    MODE_FREE = 1,  // 自由视角
    MODE_REMOTE = 2,  // 远程视角：相机固定，控制飞船
    MODE_FOLLOW = 3   // 第三人称跟随视角
};

// 飞船：完整 6 自由度 飞行，采用匀加速物理
struct Spaceship
{
    // ===== 初始位置 =====
    glm::vec3 position = glm::vec3(-100.0f, 40.0f, 60.0f);

    // ===== 朝向四元数。初始值等于旧的硬编码方向 =====
    // （绕 X 轴 -90 度，Y 轴 15 度，Z 轴 185 度），使飞船初始朝向一致。
    glm::quat heading =
        glm::angleAxis(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
        glm::angleAxis(glm::radians(15.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::angleAxis(glm::radians(185.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    // ===== 定步长物理：上一物理步的状态快照（供渲染插值用）=====
    glm::vec3 prevPosition = glm::vec3(-100.0f, 40.0f, 60.0f);
    glm::quat prevHeading =
        glm::angleAxis(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
        glm::angleAxis(glm::radians(15.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::angleAxis(glm::radians(185.0f), glm::vec3(0.0f, 0.0f, 1.0f));


    // ===== 平移：前后（沿机头方向） =====
    float speed = 0.0f;          // 当前前进速度
    float targetSpeed = 0.0f;    // 目标前进速度（由输入设置）
    float forwardMax = 30.0f;    // 最大前进速度
    float backwardMax = 10.0f;   // 最大后退速度（较小）
    float accel = 20.0f;         // 前进加速度 a
    float decel = 30.0f;         // 减速（松开油门）
    float backwardAccel = 8.0f;  // 后退加速度 a1（|a1| < a）

    // ===== 平移：垂直（沿局部上方向） =====
    float verticalSpeed = 0.0f;        // 当前垂直速度
    float targetVerticalSpeed = 0.0f;  // 目标垂直速度（由输入设置）
    float upMax = 10.0f;         // 最大上升速度
    float downMax = 10.0f;       // 最大下降速度
    float upAccel = 12.0f;       // 上升加速度
    float downAccel = 12.0f;     // 下降加速度

    // ===== 旋转：角速度 + 角加速度 =====
    float yawAV = 0.0f, targetYawAV = 0.0f;
    float pitchAV = 0.0f, targetPitchAV = 0.0f;
    float rollAV = 0.0f, targetRollAV = 0.0f;
    float turnRate = 10.0f;      // 转向速率阈值（度/秒）
    float angularAccel = 20.0f;  // 角加速度（度/秒^2）


    // ===== 函数区 =====

    // 机头在世界空间的方向
    glm::vec3 Forward() const { return heading * glm::vec3(0.0f, -1.0f, 0.0f); }
    // 局部上方向在世界空间的表示
    glm::vec3 Up() const { return heading * glm::vec3(0.0f, 0.0f, -1.0f); }

    // ===== 矩阵获取 =====
    glm::mat4 GetModelMatrix() const
    {
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, position);
        m = m * glm::mat4_cast(heading);
        m = glm::scale(m, glm::vec3(0.0005f));
        return m;
    }

    // ===== 渲染插值：a 是"当前帧落在上一个物理步区间内的比例" =====
    glm::vec3 RenderPosition(float a) const { return glm::mix(prevPosition, position, a); }
    glm::quat RenderHeading(float a) const { return glm::slerp(prevHeading, heading, a); }
    glm::vec3 RenderForward(float a) const { return RenderHeading(a) * glm::vec3(0.0f, -1.0f, 0.0f); }


    // ===== 矩阵获取 =====    
    // 渲染用模型矩阵：姿态取插值后的，其余与无参版本完全一致
    glm::mat4 GetModelMatrix(float a) const
    {
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, RenderPosition(a));
        m = m * glm::mat4_cast(RenderHeading(a));
        m = glm::scale(m, glm::vec3(0.0005f));
        return m;
    }


    // 辅助：按 rate*dt 将 cur 向 target 推近，并在 target 处截断
    static float approach(float cur, float target, float rate, float dt)
    {
        float step = rate * dt;
        if (cur < target) return glm::min(cur + step, target);
        if (cur > target) return glm::max(cur - step, target);
        return cur;
    }

    // 物理固定步长（秒）。纯解析积分，多跑几步不花钱
    static constexpr float FIXED_DT = 1.0f / 120.0f;
    static constexpr float MAX_ACCUM = 0.25f;
    
    // ===== 累加器 =====
    // 定步长推进一帧：把真实帧时间切成整数个 FIXED_DT
    // accum 由调用方持有（跨帧保留）；alpha 输出渲染插值因子 ∈ [0,1)
    // 返回本帧实际执行的物理步数
    int AdvanceFixed(float& accum, float frameDt, float& alpha)
    {
        accum += frameDt;
        if (accum > MAX_ACCUM)
        {
            accum = MAX_ACCUM;
        }
        int steps = 0;
        while (accum >= FIXED_DT)
        {
            FixedUpdate(FIXED_DT);
            accum -= FIXED_DT;
            ++steps;
        }
        alpha = accum / FIXED_DT;
        return steps;
    }

    // ===== 定步长驱动入口 =====
    // ：只允许从固定步长循环里调用
    // 先存快照再积分，于是 [prev, cur] 正好夹住一个 FIXED_DT，插值因子 a∈[0,1) 有效
    void FixedUpdate(float dt)
    {
        prevPosition = position;
        prevHeading = heading;
        Update(dt);
    }

    // ===== 战术折跃：瞬时重定位，且不可被插值 =====
    // 同时写 position 与 prevPosition。当 prevPosition == position 时，
    // 渲染插值  prev + a * (cur - prev)  对任意 a ∈ [0,1) 都退化为精确值，
    // 于是传送不可能被涂抹。只写 position 的后果是按 §5.1 的推导：偏差为
    // (1-a)*D，上确界正好是 D = 100 单位，屏幕上表现为舰体在一帧内被拉过
    // 整个折跃距离。
    // 折跃不改变 heading；prevHeading 一并同步，是为了让这个不变量在后续
    // 改动下依然成立。
    void WarpTo(const glm::vec3& target)
    {
        position = target;
        prevPosition = target;
        prevHeading = heading;
    }


    // ===== 每帧更新 =====
    // ：平滑速度、旋转并移动
    void Update(float dt)
    {
        // 平移：前后
        float fwdRate = (targetSpeed > 0.0f) ? accel
            : (targetSpeed < 0.0f) ? backwardAccel : decel;
        speed = approach(speed, targetSpeed, fwdRate, dt);

        // 平移：垂直
        float vertRate = (targetVerticalSpeed > 0.0f) ? upAccel
            : (targetVerticalSpeed < 0.0f) ? downAccel : upAccel;
        verticalSpeed = approach(verticalSpeed, targetVerticalSpeed, vertRate, dt);

        // 旋转：匀角加速度
        yawAV = approach(yawAV, targetYawAV, angularAccel, dt);
        pitchAV = approach(pitchAV, targetPitchAV, angularAccel, dt);
        rollAV = approach(rollAV, targetRollAV, angularAccel, dt);

        // 移动
        position += Forward() * speed * dt;
        position += Up() * verticalSpeed * dt;

        // 旋转（匀角速运动：积分角速度）
        glm::quat dYaw = glm::angleAxis(glm::radians(yawAV) * dt, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::quat dPitch = glm::angleAxis(glm::radians(pitchAV) * dt, glm::vec3(-1.0f, 0.0f, 0.0f));
        glm::quat dRoll = glm::angleAxis(glm::radians(rollAV) * dt, glm::vec3(0.0f, 1.0f, 0.0f));
        heading = glm::normalize(heading * dYaw * dPitch * dRoll);
    }
};
