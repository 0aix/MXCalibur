function [fXY, S, F] = Compute(XY, V, P)
    N = size(XY, 1);
    n = size(P, 1);
    S = zeros(N, 1);
    F = zeros(N, 1);
    for i = 1:N
        v = V(i);
        for j = n:-1:1
            if v >= P(j, 1)
                if j < n
                    % y_1 + (y_2 - y_1) / (x_2 - x_1) * (v - x_1)
                    S(i) = P(j, 2) + (P(j + 1, 2) - P(j, 2)) / (P(j + 1, 1) - P(j, 1)) * (V(i) - P(j, 1));
                else
                    % y_0 + (y_1 - y_0) / (x_1 - x_0) * (v - x_0)
                    S(i) = P(j - 1, 2) + (P(j, 2) - P(j - 1, 2)) / (P(j, 1) - P(j - 1, 1)) * (V(i) - P(j - 1, 1));
                end
                F(i) = j;
                break;
            end
        end
    end
    fXY = XY .* [S 1366 / 768 * S];
end

