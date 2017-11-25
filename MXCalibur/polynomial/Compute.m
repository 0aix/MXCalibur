function [fXY] = Compute(XY, V, C, R)
    S = C(1) + V .* (C(2) + V .* (C(3) + V * C(4)));
    fXY = XY .* [R(1) * S R(2) * S];
end

