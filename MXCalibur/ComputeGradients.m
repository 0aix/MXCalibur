function [DLDW, DLDB] = ComputeGradients(X, S, W, Z, M, A, HK, D)
    N = size(X{1}, 1);
    K = size(W, 2);

    T = cell(1, K + 1);
    for t = 2:K
        if t == 0
            T{t} = 1.42 * abs(S{t}).^0.42;
        else
            T{t} = ones(size(X{t})); %identity
        end
    end
    T{K + 1} = ones(size(X{K + 1}));

    DLDX = cell(1, K + 1);
    for t = 2:(K + 1)
        DLDX{t} = zeros(N, size(W{t - 1}, 2));
    end
    
    for i = 1:Z
        P = A(i):(A(i) + M(i) - 1);
        dldx = ones(1, M(i)) * X{K + 1}(P, : ) - HK(i, : );
        dldx = (dldx * dldx').^-0.5 * dldx * D(i);
        DLDX{K + 1}(P,  : ) = repmat(dldx, M(i), 1);
    end

    for t = K:-1:2
        C = DLDX{t + 1} .* T{t + 1};
        DLDX{t} = C * transpose(W{t});
    end

    DLDW = cell(1, K);
    DLDB = cell(1, K);
    for t = 1:K
       C = DLDX{t + 1} .* T{t + 1};
       %DLDWT = transpose(C) * X{t};
       %DLDW{t} = transpose(DLDWT);
       DLDW{t} = transpose(X{t}) * C;
       DLDB{t} = ones(1, N) * C;
    end
end