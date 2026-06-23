#pragma once
#include <glm/glm.hpp>
#include <vector>

class BezierCurve
{
public:
    void addPoint(glm::vec2 p) { m_pts.push_back(p); }
    void removePoint(int i) { m_pts.erase(m_pts.begin() + i); }
    void movePoint(int i, glm::vec2 p) { m_pts[i] = p; }
    void clear() { m_pts.clear(); }
    int  count() const { return (int)m_pts.size(); }
    const std::vector<glm::vec2>& points() const { return m_pts; }

    // 在最近控制点阈值内查找，返回索引，-1 表示没找到
    int  findNearest(glm::vec2 p, float radius = 0.03f) const;

    // de Casteljau 求值
    glm::vec2 eval(float t) const;

    // 均匀采样
    std::vector<glm::vec2> sample(int n = 200) const;

private:
    std::vector<glm::vec2> m_pts;
};
