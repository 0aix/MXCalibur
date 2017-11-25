[Z, M, A, N, HK, X_train, D] = LoadData('trainingdata.txt');

W = { UDistMatrix(2, 4), UDistMatrix(4, 4), UDistMatrix(4, 2) };
b = { zeros(1, 4), zeros(1, 4), zeros(1, 2) };
%W = { UDistMatrix(2, 4), UDistMatrix(4, 2) };
%b = { zeros(1, 4), zeros(1, 2) };

K = size(W, 2);

%W_ = cell(1, K);
%b_ = cell(1, K);
%l_ = Inf;
W = W_;
b = b_;

I = 0;
c = 0;

X_train = fXY;

[X, S] = ComputeLayers(X_train, W, b);

while 1
    [DLDW, DLDB] = ComputeGradients(X, S, W, Z, M, A, HK, D);
    for v = 1:K
        W{v} = W{v} - 0.1^6 * DLDW{v};
        b{v} = b{v} - 0.1^6 * DLDB{v};
    end
    I = I + 1;
    
    [X, S] = ComputeLayers(X_train, W, b);
    % Compute Loss
    l = 0;
    for i = 1:Z
        s = ones(1, M(i)) * X{K + 1}(A(i):(A(i) + M(i) - 1), : ) - HK(i, : );
        l = l + sqrt(s * s') * D(i);
    end
    
    if l < l_
        W_ = W;
        b_ = b;
        l_ = l;
    end
    
    fprintf('I: %d, l: %d, l_: %d\n', I, l, l_);
    if I > 10000
        if l == l_
            c = 0;
        elseif c == 100
            break;
        else 
           c = c + 1;
        end
    end
end

fprintf('l_: %d\n', l_);