function [A] = UDistMatrix(m, n)
    A = (rand(m, n) - 0.5) * 8 * sqrt(6 / (m + n));
end

