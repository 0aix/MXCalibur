function [AccelDataPoints, Accel, Interval] = ComputeCurve(DataPoints, Speeds, InflectionPoints)
    N = size(DataPoints, 1);
    n = size(InflectionPoints, 1);
    Accel = zeros(N, 1);
    Interval = zeros(N, 1);
    for i = 1:N
        v = Speeds(i);
        for j = n:-1:1
            if v >= InflectionPoints(j, 1)
                if j < n
                    % y_1 + (y_2 - y_1) / (x_2 - x_1) * (v - x_1)
                    Accel(i) = InflectionPoints(j, 2) + (InflectionPoints(j + 1, 2) - ...
                           InflectionPoints(j, 2)) / (InflectionPoints(j + 1, 1) - InflectionPoints(j, 1)) * ...
                           (Speeds(i) - InflectionPoints(j, 1));
                else
                    % y_0 + (y_1 - y_0) / (x_1 - x_0) * (v - x_0)
                    Accel(i) = InflectionPoints(j - 1, 2) + (InflectionPoints(j, 2) - ...
                           InflectionPoints(j - 1, 2)) / (InflectionPoints(j, 1) - InflectionPoints(j - 1, 1)) * ...
                           (Speeds(i) - InflectionPoints(j - 1, 1));
                end
                Interval(i) = j;
                break;
            end
        end
    end
    AccelDataPoints = DataPoints .* [Accel 1366 / 768 * Accel];
end

