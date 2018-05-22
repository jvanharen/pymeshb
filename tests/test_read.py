import pymeshb

#mesh = pymeshb.read('/Users/jvanhare/Codes/Reg/ALL/3D_ONERA_M6/mesh/m6_tmr_tetra_level_2.meshb')
mesh = pymeshb.read('../libmeshb/sample_meshes/quad.meshb')
print(mesh.keys())
print('mesh -->', mesh)

print('Vertices -->', mesh['Vertices'])
print('xyz -->', mesh['Vertices'][0])
print('ref -->', mesh['Vertices'][1])


print('Quadrilaterals -->', mesh['Quadrilaterals'][0])

# sol = pymeshb.read('../libmeshb/sample_meshes/out.sol')
# print(sol)
