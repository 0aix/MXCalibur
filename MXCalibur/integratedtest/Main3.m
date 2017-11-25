W = { UDistMatrix(2, 4), UDistMatrix(4, 4), UDistMatrix(4, 2) };
b = { zeros(1, 4), zeros(1, 4), zeros(1, 2) };

K = size(W, 2);

%W_ = cell(1, K);
%b_ = cell(1, K);
%loss_ = Inf;
W = W_;
b = b_;

I = 0;
c = 0;

[MAccelDataPoints, Sum] = ComputeLayers(AccelDataPoints_, W, b);

while 1
    [dLdW, dLdb] = ComputeGradients(MAccelDataPoints, Sum, W, SetCount, SetSizes, SetIndices, SetCenterPoints);
    for v = 1:K
        W{v} = W{v} - 0.1^9 * dLdW{v};
        b{v} = b{v} - 0.1^9 * dLdb{v};
    end
    I = I + 1;
    
    [MAccelDataPoints, Sum] = ComputeLayers(AccelDataPoints_, W, b);
    % Compute Loss
    loss = 0;
    for i = 1:SetCount
        s = ones(1, SetSizes(i)) * MAccelDataPoints{K + 1}(SetIndices(i):(SetIndices(i) + SetSizes(i) - 1), : ) - ...
            SetCenterPoints(i, : );
        loss = loss + sqrt(s * s');
    end
    loss = loss / SetCount;
    
    if loss < loss_
        W_ = W;
        b_ = b;
        MAccelDataPoints_ = MAccelDataPoints;
        loss_ = loss;
    end
    
    fprintf('I: %d, l: %d, l_: %d\n', I, loss, loss_);
    if I > 10000
        if loss == loss_
            c = 0;
        elseif c == 100
            break;
        else 
           c = c + 1;
        end
    end
end

fprintf('l_: %d\n', loss_);