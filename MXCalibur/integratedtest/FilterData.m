Keep = 1000;

SetLosses = zeros(SetCount, 1);
for i = 1:SetCount
    s = ones(1, SetSizes(i)) * MAccelDataPoints_{K + 1}(SetIndices(i):(SetIndices(i) + SetSizes(i) - 1), : ) - ...
        SetCenterPoints(i, : );
    SetLosses(i) = sqrt(s * s');
end

SetSorts = sortrows([SetLosses SetIndices SetSizes SetCenterPoints]);

%[SetCount, SetSizes, SetIndices, DataPointsCount, SetCenterPoints, DataPoints, Speeds
kDataPointsCount = 0;
kSetCount = 0;
for i = 1:SetCount
    kDataPointsCount = kDataPointsCount + SetSorts(i, 3);
    kSetCount = kSetCount + 1;
    if kDataPointsCount >= Keep
        break;
    end
end
kSetSizes = SetSorts(1:kSetCount, 3);
kSetIndices = SetSorts(1:kSetCount, 2);
kSetCenterPoints = SetSorts(1:kSetCount, 4);

kDataPoints = zeros(kDataPointsCount, 2);
kSpeeds = zeros(kDataPointsCount, 1);