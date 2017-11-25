function [AccelDataPoints] = ComputePoly(DataPoints, Speeds, Coefficients, Aspect)
    Accel = Coefficients(1) + Speeds .* (Coefficients(2) + Speeds .* (Coefficients(3) + Speeds * Coefficients(4)));
    AccelDataPoints = DataPoints .* [Aspect(1) * Accel Aspect(2) * Accel];
end

