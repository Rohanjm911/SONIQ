#include "BiquadFilter.hpp"
#include <algorithm>

namespace soniq::dsp {

void BiquadFilter::reset() noexcept {
    m_z1 = 0.0;
    m_z2 = 0.0;
}

void BiquadFilter::configure(FilterType type, double sampleRate, double frequency, double Q, double gainDb) {
    m_type = type;
    m_frequency = std::clamp(frequency, 10.0, sampleRate * 0.499);
    m_Q = std::clamp(Q, 0.01, 100.0);
    m_gainDb = std::clamp(gainDb, -30.0, 30.0);

    if (type == FilterType::Bypass || sampleRate <= 0.0) {
        m_b0 = 1.0;
        m_b1 = 0.0;
        m_b2 = 0.0;
        m_a1 = 0.0;
        m_a2 = 0.0;
        return;
    }

    const double omega = 2.0 * std::numbers::pi * (m_frequency / sampleRate);
    const double sinOmega = std::sin(omega);
    const double cosOmega = std::cos(omega);
    const double alpha = sinOmega / (2.0 * m_Q);
    const double A = std::pow(10.0, m_gainDb / 40.0);

    double b0 = 1.0, b1 = 0.0, b2 = 0.0, a0 = 1.0, a1 = 0.0, a2 = 0.0;

    switch (type) {
        case FilterType::LowPass:
            b0 = (1.0 - cosOmega) / 2.0;
            b1 = 1.0 - cosOmega;
            b2 = (1.0 - cosOmega) / 2.0;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cosOmega;
            a2 = 1.0 - alpha;
            break;

        case FilterType::HighPass:
            b0 = (1.0 + cosOmega) / 2.0;
            b1 = -(1.0 + cosOmega);
            b2 = (1.0 + cosOmega) / 2.0;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cosOmega;
            a2 = 1.0 - alpha;
            break;

        case FilterType::BandPass:
            b0 = alpha;
            b1 = 0.0;
            b2 = -alpha;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cosOmega;
            a2 = 1.0 - alpha;
            break;

        case FilterType::Notch:
            b0 = 1.0;
            b1 = -2.0 * cosOmega;
            b2 = 1.0;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cosOmega;
            a2 = 1.0 - alpha;
            break;

        case FilterType::PeakingEQ:
            b0 = 1.0 + alpha * A;
            b1 = -2.0 * cosOmega;
            b2 = 1.0 - alpha * A;
            a0 = 1.0 + alpha / A;
            a1 = -2.0 * cosOmega;
            a2 = 1.0 - alpha / A;
            break;

        case FilterType::LowShelf: {
            const double sqrtA = std::sqrt(A);
            const double twoSqrtAAlpha = 2.0 * sqrtA * alpha;
            b0 = A * ((A + 1.0) - (A - 1.0) * cosOmega + twoSqrtAAlpha);
            b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cosOmega);
            b2 = A * ((A + 1.0) - (A - 1.0) * cosOmega - twoSqrtAAlpha);
            a0 = (A + 1.0) + (A - 1.0) * cosOmega + twoSqrtAAlpha;
            a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cosOmega);
            a2 = (A + 1.0) + (A - 1.0) * cosOmega - twoSqrtAAlpha;
            break;
        }

        case FilterType::HighShelf: {
            const double sqrtA = std::sqrt(A);
            const double twoSqrtAAlpha = 2.0 * sqrtA * alpha;
            b0 = A * ((A + 1.0) + (A - 1.0) * cosOmega + twoSqrtAAlpha);
            b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cosOmega);
            b2 = A * ((A + 1.0) + (A - 1.0) * cosOmega - twoSqrtAAlpha);
            a0 = (A + 1.0) - (A - 1.0) * cosOmega + twoSqrtAAlpha;
            a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cosOmega);
            a2 = (A + 1.0) - (A - 1.0) * cosOmega - twoSqrtAAlpha;
            break;
        }

        default:
            break;
    }

    // Normalize by a0
    const double invA0 = 1.0 / a0;
    m_b0 = b0 * invA0;
    m_b1 = b1 * invA0;
    m_b2 = b2 * invA0;
    m_a1 = a1 * invA0;
    m_a2 = a2 * invA0;
}

double BiquadFilter::magnitudeResponse(double freq, double sampleRate) const noexcept {
    if (m_type == FilterType::Bypass || sampleRate <= 0.0) return 1.0;

    const double omega = 2.0 * std::numbers::pi * (freq / sampleRate);
    const std::complex<double> z_inv = std::polar(1.0, -omega);
    const std::complex<double> z_inv2 = z_inv * z_inv;

    const std::complex<double> num = m_b0 + m_b1 * z_inv + m_b2 * z_inv2;
    const std::complex<double> den = 1.0 + m_a1 * z_inv + m_a2 * z_inv2;

    return std::abs(num / den);
}

} // namespace soniq::dsp
