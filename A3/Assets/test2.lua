rootnode = gr.node('root')
rootnode:translate(0.0, 0.0, -100.0)
rootnode:scale( 0.01, 0.01, 0.01 )


red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
yellow = gr.material({1.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)

head = gr.mesh('ear_left_in', 'head')
rootnode:add_child(head)
head:set_material(yellow)



return rootnode