function [dLdC, dLdA] = ComputePolyGradient(SetCount, SetSizes, SetIndices, SetCenterPoints, DataPoints, Speeds, Coefficients, Aspect, AccelDataPoints)
    dLdXY = zeros(size(DataPoints));
    for i = 1:SetCount
        U = SetIndices(i):(SetIndices(i) + SetSizes(i) - 1);
        dldxy = ones(1, SetSizes(i)) * AccelDataPoints(U, : ) - SetCenterPoints(i, : );
        dldxy = (dldxy * dldxy').^-0.5 * dldxy;
        dLdXY(U,  : ) = repmat(dldxy, SetSizes(i), 1);
    end
    
    dLdA = ones(2, 1);
    dLdA(1) = sum(dLdXY( : , 1) .* AccelDataPoints( : , 1) / Aspect(1));
    dLdA(2) = sum(dLdXY( : , 2) .* AccelDataPoints( : , 2) / Aspect(2));
    dLdA = 0.1.^(floor(log10(abs(dLdA))) + 4) .* dLdA;
    dLdA = dLdA / SetCount;
    
    xy = [Aspect(1) * DataPoints( : , 1) Aspect(2) * DataPoints( : , 2)];
    dLdXY = sum(dLdXY .* xy, 2);
    
    dAdC = zeros(size(Coefficients));
    dAdC(1) = sum(dLdXY);
    dLdXY = dLdXY .* Speeds;
    dAdC(2) = sum(dLdXY);
    dLdXY = dLdXY .* Speeds;
    dAdC(3) = sum(dLdXY);
    dLdXY = dLdXY .* Speeds;
    dAdC(4) = sum(dLdXY);
    dAdC = 0.1.^(floor(log10(abs(dAdC))) + 4) .* dAdC;
    dLdC = dAdC / SetCount;
end

