module JuliaCppTests

function roundtrip(val)
  #@show val
  #println(typeof(val))
  val
end

function roundtrip2(a, b)
  #@show a, b
  #println(typeof((a, b)))
  return a, b
end

function modifyArray(val)
  reverse!(val)
end

function modifyNestedArray(val)
  for a in val
    reverse!(a)
  end
  val
end

function getArrayOfArrays()
  return Array[[5,2,9],[1,2,4]]
end

function getArrayOfArrays2()
  return Array[[2,2],[1,2,4]]
end

function getArray()
  return [23,45,67]
end

function getMultiReturn()
  return Int32(24), "tester", Float64[233.23, 2323.424221231, -2.232], Array[Array[[2],[1,4,-9]],Array[Int64[],[2,4]]]
end

function errorFunction()
  error("error")
end

function keywordArgsFunction(arg1, arg2; named1::Bool=false, named2="default")
  #println(arg1)
  #println(arg2)
  #println(named1)
  #println(named2)
  return arg1, arg2, named1, named2
end

end
