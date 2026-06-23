#include "BezierCurve.h"

int BezierCurve::findNearest(glm::vec2 p, float radius) const
{
    int best = -1;
    float bestDist = radius;
    for (int i = 0; i < (int)m_pts.size(); ++i)
    {
        float d = glm::distance(m_pts[i], p);
        if (d < bestDist) { bestDist = d; best = i; }
    }
    return best;
}

glm::vec2 BezierCurve::eval(float t) const
{
	int n = (int)m_pts.size();  // n 控制点数量
    if (n == 0) 
        return glm::vec2(0.0f);

    std::vector<glm::vec2> tmp = m_pts;
    for (int k = 1; k < n; ++k)
        for (int i = 0; i < n - k; ++i)
			tmp[i] = (1.0f - t) * tmp[i] + t * tmp[i + 1];
    return tmp[0];
}

std::vector<glm::vec2> BezierCurve::sample(int n) const
{
    std::vector<glm::vec2> out;
    if (m_pts.size() < 2) 
        return out;
    for (int i = 0; i <= n; ++i)
        out.push_back(eval((float)i / n));
    return out;
}
