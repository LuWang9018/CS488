rootnode = gr.node('root')
rootnode:translate(0.0, 0.0, 0.0)
rootnode:scale( 0.01, 0.01, 0.01 )


red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
yellow = gr.material({1.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)
black = gr.material({0.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)

body = gr.mesh('body', 'body')
rootnode:add_child(body)
body:set_material(yellow)

head_joint = gr.joint('head_joint', {-1, 0, 1}, {-1, 0, 1})
body:add_child(head_joint)
head = gr.mesh('head', 'head')
head_joint:add_child(head)
head:set_material(yellow)



ear_left_out_joint = gr.joint('ear_left_out_joint', {-1, 0, 1}, {-1, 0, 1})
head:add_child(ear_left_out_joint)
ear_left_out = gr.mesh('ear_left_out', 'ear_left_out')
ear_left_out_joint:add_child(ear_left_out)
ear_left_out:set_material(black)

ear_right_out_joint = gr.joint('ear_right_out_joint', {-1, 0, 1}, {-1, 0, 1})
head:add_child(ear_right_out_joint)
ear_right_out = gr.mesh('ear_right_out', 'ear_right_out')
ear_right_out_joint:add_child(ear_right_out)
ear_right_out:set_material(black)

ear_left_in = gr.mesh('ear_left_in', 'ear_left_in')
ear_left_out_joint:add_child(ear_left_in)
ear_left_in:set_material(yellow)

ear_right_in = gr.mesh('ear_right_in', 'ear_right_in')
ear_right_out_joint:add_child(ear_right_in)
ear_right_in:set_material(yellow)





eye_left_out_joint = gr.joint('eye_left_out_joint', {-1, 0, 1}, {-1, 0, 1})
head:add_child(eye_left_out_joint)
eye_left_out = gr.mesh('eye_left_out', 'eye_left_out')
eye_left_out_joint:add_child(eye_left_out)
eye_left_out:set_material(black)

eye_right_out_joint = gr.joint('eye_right_out_joint', {-1, 0, 1}, {-1, 0, 1})
head:add_child(eye_right_out_joint)
eye_right_out = gr.mesh('eye_right_out', 'eye_right_out')
eye_right_out_joint:add_child(eye_right_out)
eye_right_out:set_material(black)

eye_left_in_joint = gr.joint('eye_left_in_joint', {-1, 0, 1}, {-1, 0, 1})
eye_left_out:add_child(eye_left_in_joint)
eye_left_in = gr.mesh('eye_left_in', 'eye_left_in')
eye_left_in_joint:add_child(eye_left_in)
eye_left_in:set_material(white)

eye_right_in_joint = gr.joint('eye_right_in_joint', {-1, 0, 1}, {-1, 0, 1})
eye_right_out:add_child(eye_right_in_joint)
eye_right_in = gr.mesh('eye_right_in', 'eye_right_in')
eye_right_in_joint:add_child(eye_right_in)
eye_right_in:set_material(white)




face_left_joint = gr.joint('face_left_joint', {-1, 0, 1}, {-1, 0, 1})
head:add_child(face_left_joint)
face_left = gr.mesh('face_left', 'face_left')
face_left_joint:add_child(face_left)
face_left:set_material(red)

face_right_joint = gr.joint('face_right_joint', {-1, 0, 1}, {-1, 0, 1})
head:add_child(face_right_joint)
face_right = gr.mesh('face_right', 'face_right')
face_right_joint:add_child(face_right)
face_right:set_material(red)

arm_left_joint = gr.joint('arm_left_joint', {-1, 0, 1}, {-1, 0, 1})
head:add_child(arm_left_joint)
arm_left = gr.mesh('arm_left', 'arm_left')
arm_left_joint:add_child(arm_left)
arm_left:set_material(yellow)

arm_right_joint = gr.joint('arm_right_joint', {-1, 0, 1}, {-1, 0, 1})
head:add_child(arm_right_joint)
arm_right = gr.mesh('arm_right', 'arm_right')
arm_right_joint:add_child(arm_right)
arm_right:set_material(yellow)

leg_left_joint = gr.joint('leg_left_joint', {-1, 0, 1}, {-1, 0, 1})
head:add_child(leg_left_joint)
leg_left = gr.mesh('leg_left', 'leg_left')
leg_left_joint:add_child(leg_left)
leg_left:set_material(yellow)

leg_right_joint = gr.joint('leg_right_joint', {-1, 0, 1}, {-1, 0, 1})
head:add_child(leg_right_joint)
leg_right = gr.mesh('leg_right', 'leg_right')
leg_right_joint:add_child(leg_right)
leg_right:set_material(yellow)

tail_joint = gr.joint('tail_joint', {-1, 0, 1}, {-1, 0, 1})
body:add_child(tail_joint)
tail = gr.mesh('tail', 'tail')
tail_joint:add_child(tail)
tail:set_material(black)

body_up = gr.mesh('body_up', 'body_up')
body:add_child(body_up)
body_up:set_material(black)

return rootnode