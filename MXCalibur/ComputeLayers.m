function [X, S] = ComputeLayers(X1, W, b)
    N = size(X1, 1);
    K = size(W, 2);
    X = cell(1, K + 1);
    S = cell(1, K + 1);
    X{1} = X1;
    for t = 1:(K - 1)
        S{t + 1} = X{t} * W{t} + repmat(b{t}, N, 1);
        if t + 1 == 0
            X{t + 1} = sign(S{t + 1}) .* abs(S{t + 1}).^1.42;
        else
            X{t + 1} = S{t + 1}; %identity
        end
    end
    S{K + 1} = X{K} * W{K} + repmat(b{K}, N, 1);
    X{K + 1} = S{K + 1};
end