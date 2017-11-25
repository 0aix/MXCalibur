%Coefficients = [2.5957204928923185e+000; 8.3322606888998951e-001; -1.0738616803756307e-002; 4.3696740339727921e-005];
%Aspect = [1; 1366 / 768];
%loss_ = Inf;
Coefficients = Coefficients_;
Aspect = Aspect_;

[AccelDataPoints] = ComputePoly(DataPoints, Speeds, Coefficients, Aspect);

I = 0;
c = 0;

while 1
    [dLdC, dLdR] = ComputePolyGradient(SetCount, SetSizes, SetIndices, SetCenterPoints, DataPoints, Speeds, ...
                   Coefficients, Aspect, AccelDataPoints);
    Coefficients = Coefficients - 0.1^1 * 1 * dLdC;
    Aspect = Aspect - 0.1^0 * 1 * dLdR;
    I = I + 1;
    
    [AccelDataPoints] = ComputePoly(DataPoints, Speeds, Coefficients, Aspect);
    % Compute Loss
    loss = 0;
    for i = 1:SetCount
        s = ones(1, SetSizes(i)) * AccelDataPoints(SetIndices(i):(SetIndices(i) + SetSizes(i) - 1), : ) - ...
            SetCenterPoints(i, : );
        loss = loss + sqrt(s * s');
    end
    loss = loss / SetCount;
    
    if loss < loss_
        Coefficients_ = Coefficients;
        Aspect_ = Aspect;
        AccelDataPoints_ = AccelDataPoints;
        loss_ = loss;
    end
    
    fprintf('I: %d, l: %d, l_: %d\n', I, loss, loss_);
    if I > 10000
        if loss == loss_
            c = 0;
        elseif c == 10000
            break;
        else 
           c = c + 1;
        end
    end
end

fprintf('l_: %d\n', loss_);