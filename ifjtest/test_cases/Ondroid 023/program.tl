require "ifj21"

function sqrt(n: number, eps: number): number
    -- The current approximation
	local x: number = n

	if n == nil then
        return nil
    else
	end

	if n < 0 then
        return nil
    else
	end

	while 1 do
		-- Calculate next iteration to get a bit closer
		local root: number = 0.5 * (x + n/x)

		-- Creative way to check if abs(root - x) < eps
		if ((root - x) < eps) then
			if ((0 - eps) < (root - x)) then
                return x
            else
            end
        else
        end

		-- Update the approximation
		x = root
	end
end

function main()
    write ("Type your number: ")

	local n: number = readn()
	local eps: number = 0.000001

	local result: number = sqrt(n, eps)

	if result then
        write(result, "\n")
    else
        write("Could not calculate\n")
    end
end

main()
