/*  Created on: Mar 29, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "skeletons_renderable.h"

# include <graphics-origin/application/gl_window_renderer.h>
# include <graphics-origin/application/gl_helper.h>
# include <graphics-origin/application/camera.h>
# include <graphics-origin/geometry/vec.h>

# include <GL/glew.h>
BEGIN_MP_NAMESPACE
  median_skeletons_renderable::storage&
  median_skeletons_renderable::storage::operator=( storage&& other )
  {
    skeleton = std::move( other.skeleton );
    buffer_ids[ 0 ] = other.buffer_ids[ 0 ];
    buffer_ids[ 1 ] = other.buffer_ids[ 1 ];
    buffer_ids[ 2 ] = other.buffer_ids[ 2 ];
    buffer_ids[ 3 ] = other.buffer_ids[ 3 ];
    vao = other.vao;
    dirty = other.dirty;
    active = other.active;
    destroyed = other.destroyed;
    return *this;
  }

  median_skeletons_renderable::storage::storage()
    : buffer_ids{0,0,0,0}, vao{ 0 }, dirty{true}, active{ false }, destroyed{false}
  {}


  median_skeletons_renderable::median_skeletons_renderable(
      graphics_origin::application::shader_program_ptr program )
  {
    m_model = gpu_mat4(1.0);
    m_program = program;
  }

  median_skeletons_renderable::~median_skeletons_renderable()
  {
    remove_gpu_data();
  }

  void
  median_skeletons_renderable::remove_gpu_data()
  {
    auto size = m_skeletons.get_size();
    auto data = m_skeletons.data();
    for( decltype(size) i = 0; i < size; ++ i, ++data )
      {
        glcheck(glDeleteVertexArrays( 1, &data->vao ));
        glcheck(glDeleteBuffers( number_of_buffers, data->buffer_ids));
        data->vao = 0;
      }
  }

  void
  median_skeletons_renderable::update_gpu_data()
  {
    std::vector<uint32_t> indices;
    std::vector<gpu_vec4> colors;
    auto data = m_skeletons.data();
    for( size_t i = 0; i < m_skeletons.get_size();  )
      {
        if( data->destroyed )
          {
            data->destroyed = false;
            glcheck(glDeleteVertexArrays( 1, &data->vao));
            glcheck(glDeleteBuffers( number_of_buffers, data->buffer_ids ));
            m_skeletons.remove( data );
          }
        else
          {

          if( data->dirty )
            {
              if( !data->vao )
                {
                  glcheck(glGenVertexArrays( 1, &data->vao));
                  glcheck(glGenBuffers( number_of_buffers, data->buffer_ids));
                }

                const median_skeleton::atom_index nbatoms = data->skeleton.get_number_of_atoms();
                int  atom_location = m_program->get_attribute_location( "atom" );
                int color_location = m_program->get_attribute_location( "color");

                glcheck(glBindVertexArray( data->vao ));
                  glcheck(glBindBuffer( GL_ARRAY_BUFFER, data->buffer_ids[balls_vbo]));
                  glcheck(glBufferData( GL_ARRAY_BUFFER, sizeof(median_skeleton::atom) * nbatoms,
                    &data->skeleton.get_atom_by_index( 0 ), GL_STATIC_DRAW ));
                  glcheck(glEnableVertexAttribArray( atom_location ));
                  glcheck(glVertexAttribPointer( atom_location,
                    4, GL_DOUBLE, GL_FALSE,
                    0, 0 ));


                  real minr, maxr;
                  data->skeleton.compute_minmax_radii( minr, maxr );

                  colors.resize( nbatoms );
                  # pragma omp parallel for
                  for( median_skeleton::atom_index j = 0; j < nbatoms; ++ j )
                    {
                      colors[ j ] = gpu_vec4(graphics_origin::get_color( data->skeleton.get_atom_by_index( j ).w, minr, maxr ), 1.0 );
                    }
                  glcheck(glBindBuffer( GL_ARRAY_BUFFER, data->buffer_ids[colors_vbo]));
                  glcheck(glBufferData( GL_ARRAY_BUFFER, sizeof(gpu_vec4) * nbatoms, colors.data(), GL_STATIC_DRAW));
                  glcheck(glEnableVertexAttribArray( color_location ));
                  glcheck(glVertexAttribPointer( color_location,
                    4, GL_FLOAT, GL_FALSE,
                    0, 0 ));


                  const median_skeleton::link_index nblinks = data->skeleton.get_number_of_links();
                  indices.resize( nblinks * 2 );
                  # pragma omp parallel for
                  for( median_skeleton::link_index j = 0; j < nblinks; ++ j )
                    {
                      const auto& link = data->skeleton.get_link_by_index( j );
                      size_t offset = j * 2;
                      indices[ offset    ] = data->skeleton.get_index( link.h1 );
                      indices[ offset + 1] = data->skeleton.get_index( link.h2 );
                    }

                  glcheck(glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, data->buffer_ids[links_ibo]));
                  glcheck(glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW));

                  const median_skeleton::face_index nbfaces = data->skeleton.get_number_of_faces();
                  indices.resize( nbfaces * 3 );
                  # pragma omp parallel for
                  for( median_skeleton::face_index j = 0; j < nbfaces; ++ j )
                    {
                      const auto& face = data->skeleton.get_face_by_index( j );
                      size_t offset = j * 3;
                      indices[ offset     ] = data->skeleton.get_index( face.atoms[ 0 ] );
                      indices[ offset + 1 ] = data->skeleton.get_index( face.atoms[ 1 ] );
                      indices[ offset + 2 ] = data->skeleton.get_index( face.atoms[ 2 ] );
                    }

                  glcheck(glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, data->buffer_ids[faces_ibo]));
                  glcheck(glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW));
                glcheck(glBindVertexArray(0));
                data->dirty = false;
              }
          }
        ++i;
        ++data;
      }
  }

  void
  median_skeletons_renderable::do_render()
  {
    glcheck(glUniform2fv( m_program->get_uniform_location( "window_dimensions"), 1, glm::value_ptr( m_renderer->get_window_dimensions())));
    glcheck(glUniform3fv( m_program->get_uniform_location( "light_position"), 1, glm::value_ptr( m_renderer->get_camera()->get_position() )));
    glcheck(glUniformMatrix4fv( m_program->get_uniform_location( "model"), 1, GL_FALSE, glm::value_ptr( m_model )));
    glcheck(glUniformMatrix4fv( m_program->get_uniform_location( "view"), 1, GL_FALSE, glm::value_ptr( m_renderer->get_view_matrix() )));
    glcheck(glUniformMatrix4fv( m_program->get_uniform_location( "projection"), 1, GL_FALSE, glm::value_ptr( m_renderer->get_projection_matrix())));

    auto size = m_skeletons.get_size();
    auto data = m_skeletons.data();
    for( decltype(size) i = 0; i < size; ++ i, ++data )
      {
        if( data->active )
          {
            glcheck(glBindVertexArray( data->vao));
            glcheck(glDrawElements( GL_TRIANGLES, data->skeleton.get_number_of_faces() * 3, GL_UNSIGNED_INT, (void*)0));
          }
      }
    glcheck(glBindVertexArray( 0 ));
  }

  median_skeletons_renderable::skeleton_buffer::handle
  median_skeletons_renderable::add( const std::string& skeleton_filename )
  {
    auto pair = m_skeletons.create();
    pair.second.skeleton.load( skeleton_filename );
    pair.second.dirty = true;
    return pair.first;
  }

  median_skeletons_renderable::skeleton_buffer::handle
  median_skeletons_renderable::    add( median_skeleton&& skeleton )
  {
    auto pair = m_skeletons.create();
    pair.second.skeleton = std::move( skeleton );
    pair.second.dirty = true;
    return pair.first;
  }

  void median_skeletons_renderable::remove( skeleton_buffer::handle h)
  {
    m_skeletons.get( h ).destroyed = true;
  }

   median_skeletons_renderable::storage&
   median_skeletons_renderable::get( skeleton_buffer::handle h)
   {
     return m_skeletons.get( h );
   }


END_MP_NAMESPACE