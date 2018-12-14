import common

print('oneiric dependency installer')

# Header only libs
common.git_clone('https://github.com/phresnel/tukan.git', 'tukan')
common.download_file('https://github.com/catchorg/Catch2/releases/download/v2.4.2/catch.hpp', 'catchorg/catch2/catch.hpp')

common.download_file('https://raw.githubusercontent.com/nothings/stb/master/stb_image.h', 'stb/stb/stb_image.h')
common.download_file('https://raw.githubusercontent.com/nothings/stb/master/stb_image_resize.h', 'stb/stb/stb_image_resize.h')
common.download_file('https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h', 'stb/stb/stb_image_write.h')
common.download_file('https://raw.githubusercontent.com/nothings/stb/master/stb_perlin.h', 'stb/stb/stb_perlin.h')

# Boost
common.download_file('https://netcologne.dl.sourceforge.net/project/boost/boost/1.67.0/boost_1_67_0.zip', 'boost_1_67_0.zip')
common.unzip('boost_1_67_0.zip', 'boost_1_67_0')
common.start_in_folder('bootstrap.bat', where='boost_1_67_0/boost_1_67_0/', if_not_exists='project-config.jam')
common.start_in_folder('b2.exe',
                       args=[
                           '--build-type=complete',
                           # '--with-system'
                       ],
                       where='boost_1_67_0/boost_1_67_0/',
                       if_not_exists='stage')

