[SetCount, SetSizes, SetIndices, DataPointsCount, SetCenterPoints, DataPoints, Speeds] = LoadData('trainingdata.txt');

InflectionPoints = [linspace(0, 120, 25)' linspace(0, 120, 25)'];
loss_ = Inf;
%InflectionPoints = InflectionPoints_;

[AccelDataPoints, Accel, Interval] = ComputeCurve(DataPoints, Speeds, InflectionPoints);

I = 0;
c = 0;

while 1
    [dLdIP] = ComputeCurveGradient(SetCount, SetSizes, SetIndices, SetCenterPoints, DataPoints, Speeds, ...
                                  InflectionPoints, AccelDataPoints, Interval);
    InflectionPoints = InflectionPoints - 0.1^2 * dLdIP;
    I = I + 1;
    
    [AccelDataPoints, Accel, Interval] = ComputeCurve(DataPoints, Speeds, InflectionPoints);
    % Compute Loss
    loss = 0;
    for i = 1:SetCount
        s = ones(1, SetSizes(i)) * AccelDataPoints(SetIndices(i):(SetIndices(i) + SetSizes(i) - 1), : ) - ...
            SetCenterPoints(i, : );
        loss = loss + sqrt(s * s');
    end
    loss = loss / SetCount;
    
    if loss < loss_
        InflectionPoints_ = InflectionPoints;
        loss_ = loss;
    end
    
    fprintf('I: %d, loss: %d, loss_: %d\n', I, loss, loss_);
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