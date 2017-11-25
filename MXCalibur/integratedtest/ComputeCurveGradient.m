function [dLdIP] = ComputeCurveGradient(Z, M, A, HK, XY, V, P, fXY, F)
    N = size(XY, 1);
    n = size(P, 1);
    DLDXY = zeros(size(XY));
    for i = 1:Z
        U = A(i):(A(i) + M(i) - 1);
        dldxy = ones(1, M(i)) * fXY(U, : ) - HK(i, : );
        dldxy = (dldxy * dldxy').^-0.5 * dldxy;
        DLDXY(U,  : ) = repmat(dldxy, M(i), 1);
    end
    xy = [XY( : , 1) 1366 / 768 * XY( : , 2)];
    DLDXY = sum(DLDXY .* xy, 2);
    
    DFDP = zeros(size(P));
    for i = 1:N
        f = F(i);
        dldxy = DLDXY(i);
        if f < n
            p1 = (P(f + 1, 2) - P(f, 2)) / (P(f + 1, 1) - P(f, 1));
            p2 = (V(i) - P(f, 1)) / (P(f + 1, 1) - P(f, 1));
            if f > 1
                %DFDP(f, 1) = DFDP(f, 1) - p1 * (1 - p2) * dldxy;
                DFDP(f, 2) = DFDP(f, 2) + (1 - p2) * dldxy;
            end
            %DFDP(f + 1, 1) = DFDP(f + 1, 1) - p1 * p2 * dldxy;
            DFDP(f + 1, 2) = DFDP(f + 1, 2) + p2 * dldxy;
        else
            p1 = (P(f, 2) - P(f - 1, 2)) / (P(f, 1) - P(f - 1, 1));
            p2 = (V(i) - P(f - 1, 1)) / (P(f, 1) - P(f - 1, 1));
            %DFDP(f, 1) = DFDP(f, 1) - p1 * (1 - p2) * dldxy;
            DFDP(f, 2) = DFDP(f, 2) + (1 - p2) * dldxy;
        end
    end
    dLdIP = DFDP / Z;
end

