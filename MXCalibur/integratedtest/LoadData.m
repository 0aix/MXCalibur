function [SetCount, SetSizes, SetIndices, DataPointsCount, SetCenterPoints, DataPoints, Speeds] = LoadData(file_path)
    f = fopen(file_path);
    buffer = textscan(f, '%f', 1);
    SetCount = buffer{1};
    buffer = textscan(f, '%f', SetCount);
    SetSizes = buffer{1};
    SetIndices = ones(SetCount, 1);
    for i = 2:SetCount
       SetIndices(i) = SetIndices(i - 1) + SetSizes(i - 1);
    end
    DataPointsCount = sum(SetSizes);
    buffer = textscan(f, '%f %f', SetCount);
    SetCenterPoints = [buffer{1} buffer{2}];
    buffer = textscan(f, '%f %f', DataPointsCount);
    DataPoints = [buffer{1} buffer{2}];
    Speeds = sqrt(sum(DataPoints .* DataPoints, 2));
    fclose(f);
end